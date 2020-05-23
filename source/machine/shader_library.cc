#include "shader_library.h"

#include "shader_loader.h"

namespace pixel {

ShaderLibrary::ShaderLibrary(vk::Device device) : device_(device) {}

ShaderLibrary::~ShaderLibrary() = default;

bool ShaderLibrary::AddShader(const char* file_path,
                              vk::PipelineShaderStageCreateInfo shader_stage) {
  auto module = LoadShaderModule(device_, file_path);
  if (!module) {
    return false;
  }

  shader_stage.setModule(module.get());

  shader_modules_.emplace_back(std::move(module));
  pipeline_create_infos_.push_back(shader_stage);
  return true;
}

bool ShaderLibrary::AddDefaultVertexShader(const char* file_path) {
  return AddShader(file_path, {
                                  {},                                // flags
                                  vk::ShaderStageFlagBits::eVertex,  // stage
                                  nullptr,                           // module
                                  "main",  // method name
                                  nullptr  // specialization info
                              });
}

bool ShaderLibrary::AddDefaultFragmentShader(const char* file_path) {
  return AddShader(file_path, {
                                  {},                                  // flags
                                  vk::ShaderStageFlagBits::eFragment,  // stage
                                  nullptr,                             // module
                                  "main",  // method name
                                  nullptr  // specialization info
                              });
}

const std::vector<vk::PipelineShaderStageCreateInfo>&
ShaderLibrary::GetPipelineShaderStageCreateInfos() const {
  return pipeline_create_infos_;
}

}  // namespace pixel
