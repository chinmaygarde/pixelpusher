#include "image.h"

namespace pixel {

ImageView::ImageView(std::shared_ptr<Image> image, vk::UniqueImageView view)
    : image_(std::move(image)), view_(std::move(view)) {}

ImageView::~ImageView() = default;

ImageView::operator bool() const {
  return IsValid();
}

bool ImageView::IsValid() const {
  return view_ && image_;
}

const vk::ImageView* ImageView::operator->() const {
  return &view_.get();
}

vk::ImageView ImageView::GetImageView() const {
  return view_.get();
}

vk::Image ImageView::GetImage() const {
  return image_->image;
};

}  // namespace pixel
