#include "shader_module.h"

#include <optional>
#include <sstream>
#include <vector>

#include <shaderc/shaderc.hpp>

#include "file.h"
#include "filesystem_watcher.h"
#include "mapping.h"
#include "shader_location.h"
#include "vulkan.h"

namespace pixel {

static std::string GetShaderPath(const char* shader_name, bool is_spv) {
  if (shader_name == nullptr) {
    return "";
  }
  std::stringstream stream;
  stream << PIXEL_SHADERS_LOCATION << "/" << shader_name;
  if (is_spv) {
    stream << ".spv";
  }
  return stream.str();
}

static vk::UniqueShaderModule LoadShaderModuleMapping(vk::Device device,
                                                      const Mapping* mapping,
                                                      const char* debug_name) {
  if (!mapping || mapping->GetSize() == 0 || mapping->GetData() == nullptr) {
    return {};
  }

  const auto mapping_size = mapping->GetSize();

  vk::ShaderModuleCreateInfo create_info;
  create_info.setCodeSize(mapping_size);
  create_info.setPCode(reinterpret_cast<const uint32_t*>(mapping->GetData()));

  auto module = device.createShaderModuleUnique(create_info);

  if (module.result != vk::Result::eSuccess) {
    return {};
  }

  SetDebugName(device, module.value.get(), debug_name);

  return std::move(module.value);
}

static vk::UniqueShaderModule LoadShaderModuleSPIRV(vk::Device device,
                                                    const char* shader_path,
                                                    const char* debug_name) {
  return LoadShaderModuleMapping(device,                       //
                                 OpenFile(shader_path).get(),  //
                                 debug_name                    //
  );
}

static bool StringEndsWith(const std::string& string, const std::string& sub) {
  if (sub.size() > string.size()) {
    return false;
  }

  auto found = string.rfind(sub);
  if (found == std::string::npos) {
    return false;
  }

  return (string.size() - sub.size()) == found;
}

std::optional<shaderc_shader_kind> ShaderKindFromFileName(
    const char* shader_path) {
  if (shader_path == nullptr) {
    return std::nullopt;
  }

  std::string path(shader_path);

  if (StringEndsWith(path, ".vert")) {
    return shaderc_shader_kind::shaderc_glsl_vertex_shader;
  }

  if (StringEndsWith(path, ".frag")) {
    return shaderc_shader_kind::shaderc_glsl_fragment_shader;
  }

  return std::nullopt;
}

vk::UniqueShaderModule LoadShaderModuleSource(vk::Device device,
                                              const char* shader_path,
                                              const char* debug_name) {
  const auto shader_kind = ShaderKindFromFileName(shader_path);
  if (!shader_kind.has_value()) {
    return {};
  }

  auto file_mapping = OpenFile(shader_path);
  if (!file_mapping || file_mapping->GetSize() == 0) {
    return {};
  }

  shaderc::CompileOptions options;
  options.SetSourceLanguage(shaderc_source_language_glsl);
  options.SetOptimizationLevel(shaderc_optimization_level_performance);

  shaderc::Compiler compiler;
  auto compilation_result = compiler.CompileGlslToSpv(
      reinterpret_cast<const char*>(file_mapping->GetData()),  // shader source
      file_mapping->GetSize(),                                 //  source size
      shader_kind.value(),                                     // shader kind
      debug_name,                                              // debug name
      "main",                                                  // entry-point
      options                                                  // options
  );

  if (compilation_result.GetCompilationStatus() ==
      shaderc_compilation_status_success) {
    auto mapping = UnownedMapping(
        reinterpret_cast<const uint8_t*>(compilation_result.begin()),
        (compilation_result.end() - compilation_result.begin()) *
            sizeof(decltype(compilation_result)::element_type));
    return LoadShaderModuleMapping(device, mapping.get(), debug_name);
  }

  P_ERROR << "Could not compile shader: " << debug_name << " with "
          << compilation_result.GetNumErrors() << " error(s) and "
          << compilation_result.GetNumWarnings() << " warning(s).";
  P_ERROR << "Message: " << compilation_result.GetErrorMessage();
  return {};
}

std::unique_ptr<ShaderModule> ShaderModule::Load(const vk::Device& device,
                                                 const char* shader_name) {
  // Attempt loading from source files directly.
  auto shader_source_path = GetShaderPath(shader_name, false);
  if (auto src_module = LoadShaderModuleSource(
          device, shader_source_path.c_str(), shader_name)) {
    return std::unique_ptr<ShaderModule>(
        new ShaderModule(std::move(src_module), std::move(shader_source_path)));
  }

  // Attempt loading from a precompiled SPIRV file.
  if (auto spv_module = LoadShaderModuleSPIRV(
          device, GetShaderPath(shader_name, true).c_str(), shader_name)) {
    return std::unique_ptr<ShaderModule>(
        new ShaderModule(std::move(spv_module), ""));
  }

  return nullptr;
}

ShaderModule::ShaderModule(vk::UniqueShaderModule module,
                           std::string original_file_name)
    : module_(std::move(module)),
      original_file_name_(std::move(original_file_name)) {
  if (!original_file_name_.empty()) {
    FileSystemWatcher::ForProcess().WatchPathForUpdates(
        original_file_name_.c_str(), []() { P_ERROR << "File changed."; });
  }
}

ShaderModule::~ShaderModule() = default;

vk::ShaderModule ShaderModule::GetShaderModule() const {
  return module_.get();
}

std::string ShaderModule::GetOriginalFileName() const {
  return original_file_name_;
}

}  // namespace pixel
