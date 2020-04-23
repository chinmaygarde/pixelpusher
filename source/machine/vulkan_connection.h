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

  const vk::Device& GetDevice() const;

  vk::Format GetColorAttachmentFormat() const;

 private:
  vk::UniqueInstance instance_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger_;
  bool is_valid_ = false;

  bool OnDebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT severity,
      VkDebugUtilsMessageTypeFlagsEXT types,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data);

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
