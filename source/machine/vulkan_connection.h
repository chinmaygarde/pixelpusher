#pragma once

#include <vector>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class VulkanSwapchain;
class VulkanConnection {
 public:
  VulkanConnection(GLFWwindow* window);

  ~VulkanConnection();

  bool IsValid() const;

 private:
  vk::UniqueInstance instance_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
