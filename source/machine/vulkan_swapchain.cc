#include "vulkan_swapchain.h"

#include <optional>

#include "logging.h"


namespace pixel {

static std::optional<std::vector<vk::UniqueImageView>>
CreateSwapchainImageViews(const vk::Device& device,
                          const vk::SwapchainKHR& swapchain,
                          vk::Format swapchain_image_format) {
  auto swapchain_images = device.getSwapchainImagesKHR(swapchain);
  if (swapchain_images.result != vk::Result::eSuccess) {
    P_ERROR << "Could not get swapchain images.";
    return std::nullopt;
  }

  if (swapchain_images.value.size() == 0) {
    return std::nullopt;
  }

  std::vector<vk::UniqueImageView> image_views;

  vk::ImageViewCreateInfo view_create_info;

  view_create_info.setViewType(vk::ImageViewType::e2D);
  view_create_info.setFormat(swapchain_image_format);

  vk::ImageSubresourceRange range;
  range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  range.setBaseMipLevel(0u);
  range.setLevelCount(1u);
  range.setBaseArrayLayer(0u);
  range.setLayerCount(1u);

  view_create_info.setSubresourceRange(range);

  for (const auto& image : swapchain_images.value) {
    view_create_info.setImage(image);
    auto image_view = device.createImageViewUnique(view_create_info);
    if (image_view.result != vk::Result::eSuccess) {
      return std::nullopt;
    }
    image_views.emplace_back(std::move(image_view.value));
  }

  return image_views;
}

VulkanSwapchain::VulkanSwapchain(const vk::Device& device,
                                 vk::UniqueSwapchainKHR swapchain,
                                 vk::Format swapchain_image_format) {
  if (!swapchain) {
    P_ERROR << "Swapchain was invalid.";
    return;
  }

  auto image_views = CreateSwapchainImageViews(device, swapchain.get(), swapchain_image_format);

  if (!image_views.has_value()) {
    P_ERROR << "Could not create swapchain image views.";
    return;
  }

  swapchain_ = std::move(swapchain);
  image_views_ = std::move(image_views.value());
  is_valid_ = true;
}

VulkanSwapchain::~VulkanSwapchain() = default;

bool VulkanSwapchain::IsValid() const {
  return is_valid_;
}

}  // namespace pixel
