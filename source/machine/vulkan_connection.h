#pragma once

#include <memory>
#include <set>
#include <vector>

#include "macros.h"
#include "rendering_context.h"
#include "vulkan.h"
#include "vulkan_swapchain.h"

namespace pixel {

struct PhysicalDeviceSelection;

class VulkanConnection : public VulkanSwapchain::Delegate,
                         public RenderingContext::Delegate {
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

  std::shared_ptr<RenderingContext> GetRenderingContext() const;

 private:
  vk::UniqueInstance instance_;
  std::unique_ptr<PhysicalDeviceSelection> physical_device_selection_;
  vk::PhysicalDevice physical_device_;
  vk::PhysicalDeviceFeatures available_features_;
  vk::UniqueDevice device_;
  vk::SurfaceKHR surface_;
  std::shared_ptr<RenderingContext> rendering_context_;
  // TODO: Don't make the connection own the swapchain.
  std::unique_ptr<VulkanSwapchain> swapchain_;
  vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger_;
  bool is_valid_ = false;

  static bool OnDebugUtilsMessengerCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT types,
      const vk::DebugUtilsMessengerCallbackDataEXT& callback_data);

  // |VulkanSwapchain::Delegate|
  void OnSwapchainNeedsRecreation(const VulkanSwapchain& swapchain) override;

  // |RenderingContext::Delegate|
  vk::RenderPass GetOnScreenRenderPass() const override;

  // |RenderingContext::Delegate|
  size_t GetSwapchainImageCount() const override;

  // |RenderingContext::Delegate|
  vk::Extent2D GetScreenExtents() const override;

  P_DISALLOW_COPY_AND_ASSIGN(VulkanConnection);
};

}  // namespace pixel
