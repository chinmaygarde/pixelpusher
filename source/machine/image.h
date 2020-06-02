#pragma once

#include "macros.h"
#include "memory_allocator.h"
#include "vulkan.h"

namespace pixel {

class ImageView {
 public:
  ImageView(std::shared_ptr<Image> image, vk::UniqueImageView view);

  ~ImageView();

  operator bool() const;

  bool IsValid() const;

  const vk::ImageView* operator->() const;

  vk::ImageView GetImageView() const;

  vk::Image GetImage() const;

 private:
  std::shared_ptr<Image> image_;
  vk::UniqueImageView view_;

  P_DISALLOW_COPY_AND_ASSIGN(ImageView);
};

}  // namespace pixel
