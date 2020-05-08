#pragma once

#include "macros.h"
#include "vulkan.h"

namespace pixel {

vk::UniquePipelineLayout CreatePipelineLayout(vk::Device device,
    vk::ArrayProxy<vk::DescriptorSetLayout> descriptors_set_layouts);

}  // namespace pixel
