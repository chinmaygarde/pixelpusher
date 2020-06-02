#pragma once

#include <optional>
#include <vector>

#include "image.h"
#include "macros.h"
#include "rendering_context.h"
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
                  std::shared_ptr<RenderingContext> context,
                  vk::SwapchainCreateInfoKHR swapchain_create_info,
                  vk::Format swapchain_image_format);

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
  std::shared_ptr<RenderingContext> context_;
  const vk::SwapchainCreateInfoKHR swapchain_create_info_;
  const vk::Format image_format_;
  const vk::Extent2D extents_;
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<vk::UniqueImageView> image_views_;
  std::unique_ptr<ImageView> depth_stencil_image_view_;
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
