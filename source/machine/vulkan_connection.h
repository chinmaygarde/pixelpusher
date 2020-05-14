#pragma once

#include <memory>
#include <vector>

#include "imgui_connection.h"
#include "macros.h"
#include "memory_allocator.h"
#include "render_pass.h"
#include "vulkan.h"

namespace pixel {

class VulkanSwapchain;
struct PhysicalDeviceSelection;

class VulkanConnection {
 public:
  VulkanConnection(GLFWwindow* window);

  ~VulkanConnection();

  bool IsValid() const;

  vk::Instance GetInstance() const;

  vk::Device GetDevice() const;

  vk::PhysicalDevice GetPhysicalDevice() const;

  VulkanSwapchain& GetSwapchain() const;

  MemoryAllocator& GetMemoryAllocator() const;

  uint32_t GetGraphicsQueueFamilyIndex() const;

  const vk::PhysicalDeviceFeatures& GetAvailableFeatures() const;

  vk::PipelineCache GetPipelineCache() const;

  ImguiConnection& GetImguiConnection();

 private:
  vk::UniqueInstance instance_;
  std::unique_ptr<PhysicalDeviceSelection> physical_device_selection_;
  vk::PhysicalDevice physical_device_;
  vk::PhysicalDeviceFeatures available_features_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  std::unique_ptr<VulkanSwapchain> swapchain_;
  std::unique_ptr<MemoryAllocator> memory_allocator_;
  vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger_;
  vk::UniquePipelineCache pipeline_cache_;
  std::unique_ptr<ImguiConnection> imgui_connection_;
  bool is_valid_ = false;

  static bool OnDebugUtilsMessengerCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT types,
      const vk::DebugUtilsMessengerCallbackDataEXT& callback_data);

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
