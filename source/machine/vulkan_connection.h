#pragma once

#include <memory>
#include <vector>

#include "macros.h"
#include "memory_allocator.h"
#include "render_pass.h"
#include "vulkan.h"
#include "vulkan/vulkan.hpp"

namespace pixel {

class VulkanSwapchain;
struct PhysicalDeviceSelection;
class VulkanConnection {
 public:
  VulkanConnection(GLFWwindow* window);

  ~VulkanConnection();

  bool IsValid() const;

  const vk::Device& GetDevice() const;

  VulkanSwapchain& GetSwapchain() const;

  MemoryAllocator& GetMemoryAllocator() const;

  uint32_t GetGraphicsQueueFamilyIndex() const;

  const vk::PhysicalDeviceFeatures& GetAvailableFeatures() const;

 private:
  vk::UniqueInstance instance_;
  std::unique_ptr<PhysicalDeviceSelection> physical_device_selection_;
  vk::PhysicalDeviceFeatures available_features_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  std::unique_ptr<MemoryAllocator> memory_allocator_;
  vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger_;
  bool is_valid_ = false;

  static bool OnDebugUtilsMessengerCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT types,
      const vk::DebugUtilsMessengerCallbackDataEXT& callback_data);

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
