#pragma once

#include <cstdint>

namespace pixel {

struct Size {
  size_t width = 0;
  size_t height = 0;

  Size() = default;

  Size(size_t width, size_t height) : width(width), height(height) {}
};

}  // namespace pixel
