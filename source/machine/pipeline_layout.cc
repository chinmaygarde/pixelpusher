#include "pipeline_layout.h"

#include "logging.h"

namespace pixel {

vk::UniquePipelineLayout CreatePipelineLayout(
    vk::Device device,
    vk::ArrayProxy<vk::DescriptorSetLayout> descriptors_set_layouts) {
  vk::PipelineLayoutCreateInfo pipeline_layout;
  
  if (descriptors_set_layouts.size() > 0) {
    pipeline_layout.setSetLayoutCount(descriptors_set_layouts.size());
    pipeline_layout.setPSetLayouts(descriptors_set_layouts.data());
  }

  return UnwrapUniqueResult(device.createPipelineLayoutUnique(pipeline_layout));
}

}  // namespace pixel
