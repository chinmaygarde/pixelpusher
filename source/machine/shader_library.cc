#include "shader_library.h"

#include <algorithm>

namespace pixel {

ShaderLibrary::ShaderLibrary(vk::Device device) : device_(device) {}

ShaderLibrary::~ShaderLibrary() = default;

bool ShaderLibrary::AddShader(const char* shader_name,
                              vk::PipelineShaderStageCreateInfo shader_stage) {
  auto module = ShaderModule::Load(device_, shader_name);
  if (!module) {
    return false;
  }
  module->AddLiveUpdateCallback([this](const auto* module) {
    // Weaks are not necessary because we have unique ownership of all shader
    // modules and they cannot outlast us.
    this->OnShaderModuleDidUpdate(module);
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

size_t ShaderLibrary::AddLiveUpdateCallback(Closure p_callback) {
  return IdentifiableCallbacksAdd(live_update_callbacks_, p_callback);
}

bool ShaderLibrary::RemoveLiveUpdateCallback(size_t handle) {
  return IdentifiableCallbacksRemove(live_update_callbacks_, handle);
}

void ShaderLibrary::OnShaderModuleDidUpdate(const ShaderModule* module) {
  auto found = std::find_if(
      shader_modules_.begin(), shader_modules_.end(),
      [module](const auto& p_module) { return module == p_module.get(); });
  if (found == shader_modules_.end()) {
    return;
  }

  pipeline_create_infos_[found - shader_modules_.begin()].module =
      (*found)->GetShaderModule();

  IdentifiableCallbacksInvoke(live_update_callbacks_);
}

}  // namespace pixel
