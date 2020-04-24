#pragma once

#include <vector>

#include "macros.h"
#include "render_pass.h"
#include "vulkan.h"

namespace pixel {

class VulkanSwapchain;
class PhysicalDeviceSelection;
class VulkanConnection {
 public:
  VulkanConnection(GLFWwindow* window);

  ~VulkanConnection();

  bool IsValid() const;

  const vk::Device& GetDevice() const;

  const VulkanSwapchain& GetSwapchain() const;

 private:
  vk::UniqueInstance instance_;
  std::unique_ptr<PhysicalDeviceSelection> physical_device_selection_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger_;
  vk::Queue graphics_queue_;

  bool is_valid_ = false;

  static bool OnDebugUtilsMessengerCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT types,
      const vk::DebugUtilsMessengerCallbackDataEXT& callback_data);

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
