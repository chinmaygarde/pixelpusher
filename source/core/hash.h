#pragma once

#include <functional>

namespace pixel {

template <class Type>
constexpr void HashCombineSeed(std::size_t& seed, Type arg) {
  seed ^= std::hash<Type>{}(arg) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class Type, class... Rest>
constexpr void HashCombineSeed(std::size_t& seed,
                               Type arg,
                               Rest... other_args) {
  HashCombineSeed(seed, arg);
  HashCombineSeed(seed, other_args...);
}

constexpr std::size_t HashCombine() {
  return 0xdabbad00;
}

template <class... Type>
constexpr std::size_t HashCombine(Type... args) {
  std::size_t seed = HashCombine();
  HashCombineSeed(seed, args...);
  return seed;
}

}  // namespace pixel
