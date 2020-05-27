#pragma once

#include <memory>
#include <set>
#include <vector>

#include "macros.h"
#include "render_pass.h"
#include "rendering_context.h"
#include "vulkan.h"

namespace pixel {

class VulkanSwapchain;
struct PhysicalDeviceSelection;

class VulkanConnection {
 public:
  using SurfaceCallback = std::function<vk::SurfaceKHR(vk::Instance instance)>;

  VulkanConnection(PFN_vkGetInstanceProcAddr get_instance_proc_addr,
                   SurfaceCallback get_surface_proc_addr,
                   std::set<std::string> required_instance_extensions);

  ~VulkanConnection();

  bool IsValid() const;

  vk::Instance GetInstance() const;

  vk::Device GetDevice() const;

  vk::PhysicalDevice GetPhysicalDevice() const;

  VulkanSwapchain& GetSwapchain() const;

  uint32_t GetGraphicsQueueFamilyIndex() const;

  const vk::PhysicalDeviceFeatures& GetAvailableFeatures() const;

  std::shared_ptr<RenderingContext> CreateRenderingContext() const;

 private:
  vk::UniqueInstance instance_;
  std::unique_ptr<PhysicalDeviceSelection> physical_device_selection_;
  vk::PhysicalDevice physical_device_;
  vk::PhysicalDeviceFeatures available_features_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  // TODO: Don't make the connection own the swapchain.
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
