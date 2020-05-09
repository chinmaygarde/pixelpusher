#pragma once

#define VK_NO_PROTOTYPES

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace pixel {

template <class T>
inline auto UnwrapHandles(
    const vk::ArrayProxy<
        vk::UniqueHandle<T, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>>& handles) {
  std::vector<T> raw_handles;
  raw_handles.reserve(handles.size());
  for (const auto& handle : handles) {
    raw_handles.push_back(handle.get());
  }
  return raw_handles;
}

template <class T>
inline auto UnwrapResult(vk::ResultValue<T> result) {
  if (result.result != vk::Result::eSuccess) {
    return decltype(result.value){};
  }
  return std::move(result.value);
}

}  // namespace pixel
