#include "image_decoder.h"

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
  is_valid_ = true;
}

ImageDecoder::~ImageDecoder() = default;

bool ImageDecoder::IsValid() const {
  return is_valid_;
}

Size ImageDecoder::GetSize() const {
  return size_;
}

}  // namespace pixel
