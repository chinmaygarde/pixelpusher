#pragma once

#include <vector>

#include "macros.h"
#include "render_pass.h"
#include "vulkan.h"

namespace pixel {

class VulkanSwapchain;
class VulkanConnection {
 public:
  VulkanConnection(GLFWwindow* window);

  ~VulkanConnection();

  bool IsValid() const;

  const vk::Device& GetDevice() const;

  const VulkanSwapchain& GetSwapchain() const;

 private:
  vk::UniqueInstance instance_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger_;
  bool is_valid_ = false;

  static bool OnDebugUtilsMessengerCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT types,
      const vk::DebugUtilsMessengerCallbackDataEXT& callback_data);

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
