#pragma once

#include <vector>

#include "macros.h"
#include "vulkan_connection.h"

namespace pixel {

class VulkanSwapchain {
 public:
  VulkanSwapchain(const vk::Device& device,
                  vk::UniqueSwapchainKHR swapchain,
                  vk::Format swapchain_image_format);

  ~VulkanSwapchain();

  bool IsValid() const;

  vk::Format GetImageFormat() const;

 private:
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<vk::UniqueImageView> image_views_;
  const vk::Format image_format_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(VulkanSwapchain);
};

}  // namespace pixel
