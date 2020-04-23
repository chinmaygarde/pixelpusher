#include "pipeline_layout.h"

#include "logging.h"

namespace pixel {

vk::UniquePipelineLayout CreatePipelineLayout(const vk::Device& device) {
  vk::PipelineLayoutCreateInfo pipeline_layout;

  auto result = device.createPipelineLayoutUnique(pipeline_layout);
  if (result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create pipeline layout.";
    return {};
  }

  return std::move(result.value);
}

}  // namespace pixel
