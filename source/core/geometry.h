#pragma once

#include <cstdint>

namespace pixel {

struct Size {
  size_t width = 0;
  size_t height = 0;

  constexpr Size() = default;

  constexpr Size(size_t width, size_t height) : width(width), height(height) {}
};

struct Offset {
  int64_t x = 0;
  int64_t y = 0;

  constexpr Offset() = default;

  constexpr Offset(int64_t x, int64_t y) : x(x), y(y) {}
};

struct Point {
  int64_t x = 0;
  int64_t y = 0;

  Point() = default;

  Point(int64_t x, int64_t y) : x(x), y(y) {}

  constexpr Offset operator-(const Point& point) const {
    return Offset{x - point.x, y - point.y};
  }
};

}  // namespace pixel
