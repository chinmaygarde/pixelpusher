#pragma once

#include "macros.h"
#include "vulkan.h"

namespace pixel {

vk::UniquePipelineLayout CreatePipelineLayout(const vk::Device& device);

}  // namespace pixel
