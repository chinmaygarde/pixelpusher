#pragma once

#include <optional>
#include <vector>

#include "macros.h"
#include "vulkan.h"
#include "vulkan_connection.h"

namespace pixel {

class VulkanSwapchain {
 public:
  VulkanSwapchain(const vk::Device& device,
                  vk::UniqueSwapchainKHR swapchain,
                  vk::Format swapchain_image_format,
                  uint32_t graphics_queue_family_index,
                  vk::Extent2D extents);

  ~VulkanSwapchain();

  bool IsValid() const;

  const vk::RenderPass& GetRenderPass() const;

  std::optional<vk::CommandBuffer> AcquireNextCommandBuffer();

  bool SubmitCommandBuffer(vk::CommandBuffer buffer);

 private:
  vk::Device device_;
  vk::Queue graphics_queue_;
  vk::Extent2D extents_;
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
