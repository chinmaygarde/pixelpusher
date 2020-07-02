#pragma once

#include <cstdint>

namespace pixel {

struct Size {
  size_t width = 0;
  size_t height = 0;

  Size() = default;

  Size(size_t width, size_t height) : width(width), height(height) {}
};

struct Point {
  int64_t x = 0;
  int64_t y = 0;

  Point() = default;

  Point(int64_t x, int64_t y) : x(x), y(y) {}
};

struct Offset {
  int64_t x = 0;
  int64_t y = 0;

  Offset() = default;

  Offset(int64_t x, int64_t y) : x(x), y(y) {}
};

}  // namespace pixel
