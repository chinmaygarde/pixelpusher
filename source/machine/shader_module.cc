#include "shader_module.h"

#include <sstream>
#include <vector>

#include "file.h"
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

vk::UniqueShaderModule LoadShaderModuleSPIRV(const vk::Device& device,
                                             const char* shader_path) {
  auto shader_source_mapping = OpenFile(shader_path);

  if (!shader_source_mapping) {
    return {};
  }

  vk::ShaderModuleCreateInfo create_info;
  create_info.setCodeSize(shader_source_mapping->GetSize());
  create_info.setPCode(
      reinterpret_cast<const uint32_t*>(shader_source_mapping->GetData()));

  auto module = device.createShaderModuleUnique(create_info);

  if (module.result != vk::Result::eSuccess) {
    return {};
  }

  return std::move(module.value);
}

std::unique_ptr<ShaderModule> ShaderModule::Load(const vk::Device& device,
                                                 const char* shader_name) {
  const auto spriv_path = GetShaderPath(shader_name, true);
  auto module = LoadShaderModuleSPIRV(device, spriv_path.c_str());
  if (!module) {
    return nullptr;
  }
  SetDebugName(device, module.get(), shader_name);
  return std::unique_ptr<ShaderModule>(new ShaderModule(std::move(module), ""));
}

ShaderModule::ShaderModule(vk::UniqueShaderModule module,
                           std::string original_file_name)
    : module_(std::move(module)),
      original_file_name_(std::move(original_file_name)) {}

ShaderModule::~ShaderModule() = default;

vk::ShaderModule ShaderModule::GetShaderModule() const {
  return module_.get();
}

std::string ShaderModule::GetOriginalFileName() const {
  return original_file_name_;
}

}  // namespace pixel
