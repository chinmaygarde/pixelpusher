#pragma once

#include <vector>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class ShaderLibrary {
 public:
  ShaderLibrary(vk::Device device);

  ~ShaderLibrary();

  bool AddShader(const char* file_path,
                 vk::PipelineShaderStageCreateInfo shader_stage);

  bool AddDefaultVertexShader(const char* file_path);

  bool AddDefaultFragmentShader(const char* file_path);

  const std::vector<vk::PipelineShaderStageCreateInfo>&
  GetPipelineShaderStageCreateInfos() const;

 private:
  const vk::Device device_;
  std::vector<vk::UniqueShaderModule> shader_modules_;
  std::vector<vk::PipelineShaderStageCreateInfo> pipeline_create_infos_;

  P_DISALLOW_COPY_AND_ASSIGN(ShaderLibrary);
};

}  // namespace pixel
