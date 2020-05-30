#include "shader_library.h"

namespace pixel {

ShaderLibrary::ShaderLibrary(vk::Device device) : device_(device) {}

ShaderLibrary::~ShaderLibrary() = default;

bool ShaderLibrary::AddShader(const char* shader_name,
                              vk::PipelineShaderStageCreateInfo shader_stage) {
  auto module = ShaderModule::Load(device_, shader_name);
  if (!module) {
    return false;
  }
  module->AddLiveUpdateCallback([this]() {
    // Weaks are not necessary because we have unique ownership of all shader
    // modules and they cannot outlast us.
    this->OnShaderModuleDidUpdate();
  });
  shader_stage.setModule(module->GetShaderModule());
  shader_modules_.emplace_back(std::move(module));
  pipeline_create_infos_.push_back(shader_stage);
  return true;
}

bool ShaderLibrary::AddDefaultVertexShader(const char* shader_name) {
  return AddShader(shader_name, {
                                    {},                                // flags
                                    vk::ShaderStageFlagBits::eVertex,  // stage
                                    nullptr,                           // module
                                    "main",  // method name
                                    nullptr  // specialization info
                                });
}

bool ShaderLibrary::AddDefaultFragmentShader(const char* shader_name) {
  return AddShader(shader_name,
                   {
                       {},                                  // flags
                       vk::ShaderStageFlagBits::eFragment,  // stage
                       nullptr,                             // module
                       "main",                              // method name
                       nullptr  // specialization info
                   });
}

const std::vector<vk::PipelineShaderStageCreateInfo>&
ShaderLibrary::GetPipelineShaderStageCreateInfos() const {
  return pipeline_create_infos_;
}

size_t ShaderLibrary::AddLiveUpdateCallback(Closure closure) {
  IdentifiableCallback callback(closure);
  live_update_callbacks_[callback.GetIdentifier()] = callback;
  return callback.GetIdentifier();
}

bool ShaderLibrary::RemoveLiveUpdateCallback(size_t handle) {
  return live_update_callbacks_.erase(handle) == 1u;
}

void ShaderLibrary::OnShaderModuleDidUpdate() {
  for (const auto& callback : live_update_callbacks_) {
    callback.second();
  }
}

}  // namespace pixel
