#pragma once

#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#define VULKAN_HPP_NO_EXCEPTIONS

#include <vector>
#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include "macros.h"

namespace pixel {

class VulkanSwapchain;
class VulkanConnection {
 public:
  VulkanConnection(GLFWwindow* window);

  ~VulkanConnection();

  bool IsValid() const;

 private:
  vk::UniqueInstance instance_;
  vk::UniqueSurfaceKHR surface_;
  vk::UniqueDevice device_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
