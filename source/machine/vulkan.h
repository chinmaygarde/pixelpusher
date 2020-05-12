#pragma once

#include <stdint.h>
#include "logging.h"
#include "macros.h"
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

template <class T>
inline auto SetDebugName(vk::Device device, T object, const char* name) {
  vk::DebugUtilsObjectNameInfoEXT object_name;
  object_name.setObjectHandle(reinterpret_cast<uint64_t>(
      static_cast<typename decltype(object)::CType>(object)));
  object_name.setObjectType(decltype(object)::objectType);
  object_name.setPObjectName(name);

  if (device.setDebugUtilsObjectNameEXT(object_name) != vk::Result::eSuccess) {
    P_ERROR << "Could not attach debug name to object";
    return false;
  }
  return true;
}

struct AutoDebugMarkerEnd {
  AutoDebugMarkerEnd(vk::CommandBuffer buffer) : buffer(buffer) {}

  ~AutoDebugMarkerEnd() { buffer.debugMarkerEndEXT(); }

 private:
  vk::CommandBuffer buffer;

  P_DISALLOW_COPY_AND_ASSIGN(AutoDebugMarkerEnd);
};

[[nodiscard]] inline AutoDebugMarkerEnd DebugMarkerBegin(
    vk::CommandBuffer command_buffer,
    const char* name) {
  vk::DebugMarkerMarkerInfoEXT marker;
  marker.setPMarkerName(name);
  marker.setColor({0.0f, 1.0f, 0.0f, 1.0f});
  command_buffer.debugMarkerBeginEXT(marker);

  return AutoDebugMarkerEnd{command_buffer};
}

}  // namespace pixel
