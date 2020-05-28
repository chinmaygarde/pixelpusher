#pragma once

#include <optional>
#include <vector>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class VulkanSwapchain {
 public:
  class Delegate {
   public:
    virtual void OnSwapchainNeedsRecreation(
        const VulkanSwapchain& swapchain) = 0;
  };

  static vk::Extent2D ChooseSwapExtents(
      const vk::SurfaceCapabilitiesKHR& capabilities);

  VulkanSwapchain(Delegate& delegate,
                  vk::Device device,
                  vk::SwapchainCreateInfoKHR swapchain_create_info,
                  vk::Format swapchain_image_format,
                  uint32_t graphics_queue_family_index,
                  vk::Queue graphics_queue);

  VulkanSwapchain(const VulkanSwapchain& swapchain, vk::Extent2D new_extents);

  ~VulkanSwapchain();

  bool IsValid() const;

  const vk::Extent2D& GetExtents() const;

  const vk::RenderPass& GetRenderPass() const;

  std::optional<vk::CommandBuffer> AcquireNextCommandBuffer();

  enum class SubmitResult {
    kFailure,
    kSuccess,
    kTryAgain,
  };
  [[nodiscard]] SubmitResult SubmitCommandBuffer(vk::CommandBuffer buffer);

  size_t GetImageCount() const;

  void Retire();

 private:
  Delegate& delegate_;
  const vk::Device device_;
  const vk::SwapchainCreateInfoKHR swapchain_create_info_;
  const vk::Format image_format_;
  const uint32_t graphics_queue_family_index_;
  const vk::Queue graphics_queue_;
  const vk::Extent2D extents_;
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<vk::UniqueImageView> image_views_;
  std::vector<vk::UniqueFramebuffer> frame_buffers_;
  vk::UniqueRenderPass render_pass_;
  vk::UniqueCommandPool command_pool_;
  std::vector<vk::UniqueCommandBuffer> command_buffers_;
  vk::UniqueSemaphore ready_to_render_semaphore_;
  vk::UniqueSemaphore ready_to_present_semaphore_;
  std::optional<vk::CommandBuffer> pending_command_buffer_;
  std::optional<uint32_t> pending_swapchain_image_index_;
  bool is_valid_ = false;

  bool PrepareCommandBuffer(vk::CommandBuffer buffer, size_t swapchain_index);

  bool FinalizeCommandBuffer(vk::CommandBuffer buffer);

  P_DISALLOW_COPY_AND_ASSIGN(VulkanSwapchain);
};

}  // namespace pixel
