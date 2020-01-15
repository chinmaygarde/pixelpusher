#include "shader_loader.h"

#include <sstream>
#include <vector>

#include "shader_location.h"
#include "file.h"

namespace pixel {

std::vector<char> LoadShader(const char* shader_name) {
  if (shader_name == nullptr) {
    return {};
  }
  std::stringstream stream;
  stream << PIXEL_SHADERS_LOCATION << "/" << shader_name;
  return ReadFile(stream.str().c_str());
}

vk::UniqueShaderModule LoadShaderModule(const vk::Device& device, const char* shader_name) {
  auto shader_source = LoadShader(shader_name);
  if (shader_source.size() == 0) {
    return {};
  }
  
  vk::ShaderModuleCreateInfo create_info;
  create_info.setCodeSize(shader_source.size());
  create_info.setPCode(reinterpret_cast<const uint32_t *>(shader_source.data()));
  
  auto module = device.createShaderModuleUnique(create_info);
  
  if (module.result != vk::Result::eSuccess) {
    return {};
  }
  
  return std::move(module.value);
}

}  // namespace pixel
