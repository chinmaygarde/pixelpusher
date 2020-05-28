#include "shader_loader.h"

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
                                             const char* shader_name) {
  auto shader_source_mapping =
      OpenFile(GetShaderPath(shader_name, true).c_str());

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

  SetDebugName(device, module.value.get(), shader_name);

  return std::move(module.value);
}

vk::UniqueShaderModule LoadShaderModule(const vk::Device& device,
                                        const char* shader_name) {
  return LoadShaderModuleSPIRV(device, shader_name);
}

}  // namespace pixel
