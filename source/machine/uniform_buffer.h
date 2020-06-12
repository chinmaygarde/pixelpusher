#pragma once

#include <memory>

#include "macros.h"
#include "memory_allocator.h"

namespace pixel {

template <class T>
struct UniformBuffer {
  T prototype = {};
  std::unique_ptr<Buffer> buffer;

  UniformBuffer() = default;

  UniformBuffer(MemoryAllocator& allocator,
                T object,
                size_t copies,
                const char* debug_name)
      : prototype(std::move(object)),
        buffer(allocator.CreateHostVisibleBuffer(
            vk::BufferUsageFlagBits::eUniformBuffer,
            sizeof(T) * copies,
            debug_name)),
        copies(copies),
        current_index(copies - 1u) {
    P_ASSERT(copies > 0u);
  };

  bool UpdateUniformData() {
    BufferMapping mapping(*buffer);
    if (!mapping.IsValid()) {
      return false;
    }
    current_index = (current_index + 1) % copies;
    memcpy(static_cast<uint8_t*>(mapping.GetMapping()) +
               (sizeof(T) * current_index),
           &prototype, sizeof(T));
    return true;
  }

  operator bool() const { return static_cast<bool>(buffer); }

  T* operator->() { return &prototype; };

  std::vector<vk::DescriptorBufferInfo> GetBufferInfos() const {
    std::vector<vk::DescriptorBufferInfo> infos;
    infos.reserve(copies);
    for (size_t i = 0; i < copies; i++) {
      infos.push_back(GetBufferInfo(i));
    }
    return infos;
  }

  vk::DescriptorBufferInfo GetBufferInfo(size_t index) const {
    vk::DescriptorBufferInfo info;
    info.setBuffer(buffer->buffer);
    info.setOffset(sizeof(T) * index);
    info.setRange(sizeof(T));
    return info;
  }

  size_t GetCurrentIndex() const { return current_index; }

 private:
  size_t copies = 0;
  size_t current_index = 0;
};

}  // namespace pixel
