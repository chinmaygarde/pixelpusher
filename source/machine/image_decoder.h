#pragma once

#include <memory>

#include "command_pool.h"
#include "geometry.h"
#include "macros.h"
#include "mapping.h"
#include "memory_allocator.h"
#include "vulkan.h"

namespace pixel {

class ImageView {
 public:
  ImageView(std::shared_ptr<Image> image, vk::UniqueImageView view)
      : image_(std::move(image)), view_(std::move(view)) {}

  ~ImageView() = default;

  operator bool() const { return IsValid(); }

  bool IsValid() const { return view_ && image_; }

  const vk::ImageView* operator->() const { return &view_.get(); }

  const vk::ImageView& GetImageView() const { return view_.get(); }

  const vk::Image& GetImage() const { return image_->image; };

 private:
  std::shared_ptr<Image> image_;
  vk::UniqueImageView view_;

  P_DISALLOW_COPY_AND_ASSIGN(ImageView);
};

class ImageDecoder {
 public:
  ImageDecoder(const Mapping& mapping);

  ~ImageDecoder();

  bool IsValid() const;

  Size GetSize() const;

  std::unique_ptr<ImageView> CreateDeviceLocalImageCopy(
      MemoryAllocator& allocator,
      const CommandPool& pool,
      const char* debug_name,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done) const;

 private:
  std::shared_ptr<void> image_;
  int stb_format_ = 0;
  size_t image_data_size_ = 0;
  Size size_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(ImageDecoder);
};

}  // namespace pixel
