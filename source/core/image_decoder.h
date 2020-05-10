#pragma once

#include "macros.h"

namespace pixel {

class ImageDecoder {
 public:
  ImageDecoder();

  ~ImageDecoder();

 private:
  P_DISALLOW_COPY_AND_ASSIGN(ImageDecoder);
};

}  // namespace pixel
