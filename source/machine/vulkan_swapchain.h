#pragma once

#include <vector>

#include "macros.h"
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

 private:
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<vk::UniqueImageView> image_views_;
  std::vector<vk::UniqueFramebuffer> frame_buffers_;
  vk::UniqueRenderPass render_pass_;
  vk::UniqueCommandPool command_pool_;
  std::vector<vk::UniqueCommandBuffer> command_buffers_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(VulkanSwapchain);
};

}  // namespace pixel
