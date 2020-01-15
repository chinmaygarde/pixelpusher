#pragma once

#include "vulkan.h"

namespace pixel {

vk::UniqueShaderModule LoadShaderModule(const vk::Device& device, const char* shader_name);

}  // namespace pixel
