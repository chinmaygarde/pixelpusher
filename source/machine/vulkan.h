#pragma once

#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#define VULKAN_HPP_NO_EXCEPTIONS

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace pixel {

bool InitVulkan();

}  // namespace pixel
