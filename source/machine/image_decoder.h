#pragma once

#include <memory>

#include "geometry.h"
#include "macros.h"
#include "mapping.h"

namespace pixel {

class ImageDecoder {
 public:
  ImageDecoder(const Mapping& mapping);

  ~ImageDecoder();

  bool IsValid() const;

  Size GetSize() const;

 private:
  std::shared_ptr<void> image_;
  Size size_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(ImageDecoder);
};

}  // namespace pixel
