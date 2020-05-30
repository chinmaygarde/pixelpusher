#pragma once

#include <map>
#include <vector>

#include "closure.h"
#include "macros.h"
#include "shader_module.h"
#include "vulkan.h"

namespace pixel {

class ShaderLibrary {
 public:
  ShaderLibrary(vk::Device device);

  ~ShaderLibrary();

  bool AddShader(const char* shader_name,
                 vk::PipelineShaderStageCreateInfo shader_stage);

  bool AddDefaultVertexShader(const char* shader_name);

  bool AddDefaultFragmentShader(const char* shader_name);

  const std::vector<vk::PipelineShaderStageCreateInfo>&
  GetPipelineShaderStageCreateInfos() const;

  size_t AddLiveUpdateCallback(Closure callback);

  bool RemoveLiveUpdateCallback(size_t handle);

 private:
  const vk::Device device_;
  std::vector<std::unique_ptr<ShaderModule>> shader_modules_;
  std::vector<vk::PipelineShaderStageCreateInfo> pipeline_create_infos_;
  IdentifiableClosures live_update_callbacks_;

  void OnShaderModuleDidUpdate(const ShaderModule* module);

  P_DISALLOW_COPY_AND_ASSIGN(ShaderLibrary);
};

}  // namespace pixel
