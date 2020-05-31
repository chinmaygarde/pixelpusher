#include "shader_module.h"

#include <optional>
#include <sstream>
#include <vector>

#include <shaderc/shaderc.hpp>

#include "event_loop.h"
#include "file.h"
#include "filesystem_watcher.h"
#include "mapping.h"
#include "shader_location.h"
#include "string_utils.h"
#include "thread.h"
#include "vulkan.h"

namespace pixel {

static std::string DebugLabelFromShaderPath(const std::filesystem::path& path,
                                            bool is_spv) {
  std::stringstream stream;
  const auto filename_label = path.filename().u8string();
  stream << reinterpret_cast<const char*>(filename_label.c_str());
  if (is_spv) {
    stream << "(spirv)";
  } else {
    stream << "(source)";
  }
  return stream.str();
}

static std::filesystem::path GetShaderPath(const char* shader_name,
                                           bool is_spv,
                                           bool is_link) {
  if (shader_name == nullptr) {
    return "";
  }

  std::filesystem::path path;
  path /= PIXEL_SHADERS_LOCATION;
  path /= shader_name;
  if (is_spv) {
    path += ".spv";
  }
  if (is_link) {
    path += ".link";
  }
  path.make_preferred();
  return path;
}

static vk::UniqueShaderModule LoadShaderModuleMapping(
    vk::Device device,
    const Mapping* mapping,
    const std::string& debug_name) {
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

  SetDebugName(device, module.value.get(), debug_name.c_str());

  return std::move(module.value);
}

static vk::UniqueShaderModule LoadShaderModuleSPIRV(
    vk::Device device,
    const std::filesystem::path& path) {
  return LoadShaderModuleMapping(device,                               //
                                 OpenFile(path).get(),                 //
                                 DebugLabelFromShaderPath(path, true)  //
  );
}

std::optional<shaderc_shader_kind> ShaderKindFromFileName(
    const std::filesystem::path& shader_path) {
  if (shader_path.extension() == ".vert") {
    return shaderc_shader_kind::shaderc_glsl_vertex_shader;
  }

  if (shader_path.extension() == ".frag") {
    return shaderc_shader_kind::shaderc_glsl_fragment_shader;
  }

  return std::nullopt;
}

vk::UniqueShaderModule LoadShaderModuleSource(
    vk::Device device,
    const std::filesystem::path& shader_path) {
  const auto shader_kind = ShaderKindFromFileName(shader_path);
  if (!shader_kind.has_value()) {
    return {};
  }

  auto file_mapping = OpenFile(shader_path.c_str());
  if (!file_mapping || file_mapping->GetSize() == 0) {
    return {};
  }

  shaderc::CompileOptions options;
  options.SetSourceLanguage(shaderc_source_language_glsl);
  options.SetOptimizationLevel(shaderc_optimization_level_zero);

  const auto debug_label = DebugLabelFromShaderPath(shader_path, false);

  shaderc::Compiler compiler;
  auto compilation_result = compiler.CompileGlslToSpv(
      reinterpret_cast<const char*>(file_mapping->GetData()),  // shader source
      file_mapping->GetSize(),                                 // source size
      shader_kind.value(),                                     // shader kind
      reinterpret_cast<const char*>(
          shader_path.filename().u8string().c_str()),  // debug name
      "main",                                          // entry-point
      options                                          // options
  );

  if (compilation_result.GetCompilationStatus() ==
      shaderc_compilation_status_success) {
    auto mapping = UnownedMapping(
        reinterpret_cast<const uint8_t*>(compilation_result.begin()),
        (compilation_result.end() - compilation_result.begin()) *
            sizeof(decltype(compilation_result)::element_type));
    return LoadShaderModuleMapping(device, mapping.get(), debug_label);
  }

  P_ERROR << "Could not compile shader: " << debug_label << " with "
          << compilation_result.GetNumErrors() << " error(s) and "
          << compilation_result.GetNumWarnings() << " warning(s).";
  P_ERROR << "Message: " << compilation_result.GetErrorMessage();
  return {};
}

static std::filesystem::path LinkFromFile(
    std::filesystem::path link_file_path) {
  auto mapping = OpenFile(link_file_path);
  if (!mapping) {
    return {};
  }

  std::string file_contents(reinterpret_cast<const char*>(mapping->GetData()),
                            mapping->GetSize());
  file_contents = TrimString(file_contents);
  if (file_contents.empty()) {
    return {};
  }

  std::filesystem::path path(file_contents);
  path.make_preferred();
  return path;
}

std::unique_ptr<ShaderModule> ShaderModule::Load(vk::Device device,
                                                 const char* shader_name) {
  // Try to get the shader source path from a link file.
  auto shader_source_path =
      LinkFromFile(GetShaderPath(shader_name, false, true));

  if (shader_source_path.empty()) {
    // Attempt loading from source files directly.
    shader_source_path = GetShaderPath(shader_name, false, false);
  }

  if (auto src_module = LoadShaderModuleSource(device, shader_source_path)) {
    return std::unique_ptr<ShaderModule>(new ShaderModule(
        device, std::move(src_module), std::move(shader_source_path)));
  }

  // Attempt loading from a precompiled SPIRV file.
  if (auto spv_module = LoadShaderModuleSPIRV(
          device, GetShaderPath(shader_name, true, false))) {
    return std::unique_ptr<ShaderModule>(
        new ShaderModule(device, std::move(spv_module), ""));
  }

  return nullptr;
}

static Closure RethreadCallback(Closure closure) {
  auto dispatcher = EventLoop::ForCurrentThread().GetDispatcher();
  return [closure, dispatcher]() { dispatcher->PostTask(closure); };
}

ShaderModule::ShaderModule(vk::Device device,
                           vk::UniqueShaderModule module,
                           std::filesystem::path original_file_path)
    : device_(device),
      module_(std::move(module)),
      original_file_path_(std::move(original_file_path)),
      fs_watcher_handler_(FileSystemWatcher::ForProcess().WatchPathForUpdates(
          original_file_path_,
          RethreadCallback([&]() { OnShaderFileDidUpdate(); }))),
      weak_factory_(this) {}

ShaderModule::~ShaderModule() {
  FileSystemWatcher::ForProcess().StopWatchingForUpdates(fs_watcher_handler_);
}

vk::ShaderModule ShaderModule::GetShaderModule() const {
  return module_.get();
}

void ShaderModule::OnShaderFileDidUpdate() {
  const auto now = std::chrono::high_resolution_clock::now();
  if (now - last_shader_update_ > std::chrono::milliseconds(125)) {
    last_shader_update_ = now;
    Thread::PostBackgroundTask([device = device_,                      //
                                path = original_file_path_,            //
                                weak = weak_factory_.CreateWeakPtr(),  //
                                dispatcher =
                                    EventLoop::GetCurrentThreadDispatcher()  //
    ]() {
      auto module = LoadShaderModuleSource(device, path);
      if (!module) {
        // Nothing to do. Compilation on the updated shader failed.
        P_ERROR
            << "Shader was updated but could not be re-compiled successfully.";
        return;
      }

      dispatcher->PostTask(
          MakeCopyable([weak, module = std::move(module)]() mutable {
            if (weak) {
              weak.get()->OnShaderModuleDidUpdate(std::move(module));
            }
          }));
    });
  }
}

void ShaderModule::OnShaderModuleDidUpdate(vk::UniqueShaderModule module) {
  module_ = std::move(module);

  IdentifiableCallbacksInvoke(live_update_callbacks_, this);
}

size_t ShaderModule::AddLiveUpdateCallback(
    std::function<void(const ShaderModule*)> closure) {
  return IdentifiableCallbacksAdd(live_update_callbacks_, closure);
}

bool ShaderModule::RemoveLiveUpdateCallback(size_t handle) {
  return IdentifiableCallbacksRemove(live_update_callbacks_, handle);
}

}  // namespace pixel
