#include "image_decoder.h"

#include <memory>
#include <optional>

#include "closure.h"
#include "image.h"
#include "logging.h"
#include "vulkan.h"

GCC_PRAGMA("GCC diagnostic push")
GCC_PRAGMA("GCC diagnostic ignored \"-Wunused-function\"")
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
GCC_PRAGMA("GCC diagnostic pop")

namespace pixel {

ImageDecoder::ImageDecoder(const Mapping& mapping) {
  if (mapping.GetData() == nullptr) {
    return;
  }

  int x = 0;
  int y = 0;
  int channels_in_file = 0;

  auto pixels =
      ::stbi_load_from_memory(mapping.GetData(), mapping.GetSize(), &x, &y,
                              &channels_in_file, STBI_rgb_alpha);

  if (pixels == nullptr) {
    return;
  }

  auto image = std::shared_ptr<void>(pixels, [](void* pixels) {
    if (pixels) {
      stbi_image_free(pixels);
    }
  });

  if (x < 0 || y < 0) {
    return;
  }

  size_ = {static_cast<size_t>(x), static_cast<size_t>(y)};
  image_ = image;
  stb_format_ = STBI_rgb_alpha;
  image_data_size_ = x * y * stb_format_;
  is_valid_ = true;
}

ImageDecoder::~ImageDecoder() = default;

bool ImageDecoder::IsValid() const {
  return is_valid_;
}

Size ImageDecoder::GetSize() const {
  return size_;
}

static std::optional<vk::Format> FormatForSTBImageFormat(int format) {
  // TODO: The availability of these formats must be checked.
  switch (format) {
    case STBI_rgb_alpha:
      return vk::Format::eR8G8B8A8Srgb;
  }
  return std::nullopt;
}

std::unique_ptr<ImageView> ImageDecoder::CreateDeviceLocalImageCopy(
    MemoryAllocator& allocator,
    const CommandPool& pool,
    const char* debug_name,
    vk::ArrayProxy<vk::Semaphore> wait_semaphores,
    vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
    vk::ArrayProxy<vk::Semaphore> signal_semaphores,
    std::function<void(void)> on_done) const {
  if (!is_valid_ || !allocator.IsValid()) {
    return nullptr;
  }

  auto format = FormatForSTBImageFormat(stb_format_);
  if (!format.has_value()) {
    P_ERROR << "Unsupported image format.";
    return nullptr;
  }

  auto on_transfer_done = [image = image_, on_done]() mutable {
    image.reset();
    if (on_done) {
      on_done();
    }
  };

  vk::ImageCreateInfo image_info;

  image_info.setFormat(format.value());
  image_info.setTiling(vk::ImageTiling::eOptimal);
  image_info.setArrayLayers(1u);
  image_info.setExtent({static_cast<uint32_t>(size_.width),
                        static_cast<uint32_t>(size_.height), 1u});
  image_info.setMipLevels(1u);
  image_info.setUsage(vk::ImageUsageFlagBits::eTransferDst |
                      vk::ImageUsageFlagBits::eSampled);
  image_info.setImageType(vk::ImageType::e2D);
  image_info.setInitialLayout(vk::ImageLayout::eUndefined);
  image_info.setSharingMode(vk::SharingMode::eExclusive);
  image_info.setSamples(vk::SampleCountFlagBits::e1);

  auto image =
      allocator.CreateDeviceLocalImageCopy(image_info,                    //
                                           image_.get(),                  //
                                           image_data_size_,              //
                                           pool,                          //
                                           debug_name,                    //
                                           std::move(wait_semaphores),    //
                                           std::move(wait_stages),        //
                                           std::move(signal_semaphores),  //
                                           on_transfer_done               //
      );

  if (!image) {
    return nullptr;
  }

  vk::ImageViewCreateInfo image_view_info;
  image_view_info.setImage(image->image);
  image_view_info.setFormat(format.value());
  image_view_info.setViewType(vk::ImageViewType::e2D);
  vk::ImageSubresourceRange subresource_range;
  subresource_range.setLayerCount(1u);
  subresource_range.setLevelCount(1u);
  subresource_range.setBaseMipLevel(0u);
  subresource_range.setBaseArrayLayer(0u);
  subresource_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  image_view_info.setSubresourceRange(subresource_range);

  auto image_view =
      UnwrapResult(pool.GetDevice().createImageViewUnique(image_view_info));
  if (!image_view) {
    return nullptr;
  }

  SetDebugNameF(pool.GetDevice(), image_view.get(), "%s Image View",
                debug_name);

  return std::make_unique<ImageView>(std::move(image), std::move(image_view));
}

}  // namespace pixel
