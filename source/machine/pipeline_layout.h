#pragma once

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class PipelineLayoutBuilder {
 public:
  PipelineLayoutBuilder();

  ~PipelineLayoutBuilder();

  PipelineLayoutBuilder& AddDescriptorSetLayout(vk::DescriptorSetLayout layout);

  PipelineLayoutBuilder& AddPushConstantRange(vk::PushConstantRange range);

  vk::UniquePipelineLayout CreatePipelineLayout(vk::Device device,
                                                const char* debug_name) const;

 private:
  std::vector<vk::DescriptorSetLayout> descriptor_set_layouts_;
  std::vector<vk::PushConstantRange> push_constant_ranges_;

  P_DISALLOW_COPY_AND_ASSIGN(PipelineLayoutBuilder);
};

// TODO: Deprecate
vk::UniquePipelineLayout CreatePipelineLayout(
    vk::Device device,
    vk::ArrayProxy<vk::DescriptorSetLayout> descriptors_set_layouts);

}  // namespace pixel
