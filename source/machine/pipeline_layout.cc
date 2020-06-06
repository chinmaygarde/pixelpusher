#include "pipeline_layout.h"

#include "logging.h"

namespace pixel {

PipelineLayoutBuilder::PipelineLayoutBuilder() = default;

PipelineLayoutBuilder::~PipelineLayoutBuilder() = default;

PipelineLayoutBuilder& PipelineLayoutBuilder::AddDescriptorSetLayout(
    vk::DescriptorSetLayout layout) {
  descriptor_set_layouts_.push_back(layout);
  return *this;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::AddPushConstantRange(
    vk::PushConstantRange range) {
  push_constant_ranges_.push_back(range);
  return *this;
}

// TODO: Deprecate
vk::UniquePipelineLayout CreatePipelineLayout(
    vk::Device device,
    vk::ArrayProxy<vk::DescriptorSetLayout> descriptors_set_layouts) {
  vk::PipelineLayoutCreateInfo pipeline_layout;

  if (descriptors_set_layouts.size() > 0) {
    pipeline_layout.setSetLayoutCount(descriptors_set_layouts.size());
    pipeline_layout.setPSetLayouts(descriptors_set_layouts.data());
  }

  return UnwrapResult(device.createPipelineLayoutUnique(pipeline_layout));
}

vk::UniquePipelineLayout PipelineLayoutBuilder::CreatePipelineLayout(
    vk::Device device,
    const char* debug_name) const {
  vk::PipelineLayoutCreateInfo pipeline_layout = {
      {},                                                     // flags
      static_cast<uint32_t>(descriptor_set_layouts_.size()),  // layouts size
      descriptor_set_layouts_.data(),                         // layouts
      static_cast<uint32_t>(
          push_constant_ranges_.size()),  // push constants size
      push_constant_ranges_.data()        // push constants
  };
  auto result =
      UnwrapResult(device.createPipelineLayoutUnique(pipeline_layout));

  if (!result) {
    return {};
  }

  SetDebugNameF(device, result.get(), "%s Pipeline Layout", debug_name);
}

}  // namespace pixel
