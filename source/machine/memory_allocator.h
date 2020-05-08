#pragma once

#include <vk_mem_alloc.h>

#include <memory>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

struct Buffer {
  vk::Buffer buffer = {};
  VmaAllocator allocator = nullptr;
  VmaAllocation allocation = nullptr;

  Buffer(vk::Buffer buffer, VmaAllocator allocator, VmaAllocation allocation);

  ~Buffer();

  P_DISALLOW_COPY_AND_ASSIGN(Buffer);
};

class BufferMapping {
 public:
  BufferMapping(const Buffer& buffer) : buffer_(buffer) {
    void* mapping = nullptr;
    if (vmaMapMemory(buffer_.allocator, buffer_.allocation, &mapping) !=
        VK_SUCCESS) {
      return;
    }

    mapping_ = mapping;
    is_valid_ = true;
  }

  ~BufferMapping() {
    if (!IsValid()) {
      return;
    }

    vmaUnmapMemory(buffer_.allocator, buffer_.allocation);
  }

  void* GetMapping() const {
    P_ASSERT(IsValid());
    return mapping_;
  }

  bool IsValid() const { return is_valid_; }

  operator bool() const { return IsValid(); }

 private:
  const Buffer& buffer_;
  void* mapping_ = nullptr;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(BufferMapping);
};

class CommandPool;
class MemoryAllocator {
 public:
  MemoryAllocator(const vk::PhysicalDevice& physical_device,
                  vk::Device logical_device);

  ~MemoryAllocator();

  bool IsValid() const;

  std::unique_ptr<Buffer> CreateBuffer(
      const vk::BufferCreateInfo& buffer_info,
      const VmaAllocationCreateInfo& allocation_info);

  std::unique_ptr<Buffer> CreateHostVisibleBufferCopy(
      vk::BufferUsageFlags usage,
      const void* buffer,
      size_t buffer_size);

  std::unique_ptr<Buffer> CreateHostVisibleBuffer(vk::BufferUsageFlags usage,
                                                  size_t buffer_size);

  std::unique_ptr<Buffer> CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlags usage,
      const void* buffer,
      size_t buffer_size,
      const CommandPool& pool,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done);

 private:
  vk::Device device_;
  VmaVulkanFunctions proc_table_;
  VmaAllocator allocator_ = nullptr;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(MemoryAllocator);
};

template <class T>
struct UniformBuffer {
  T prototype = {};
  std::unique_ptr<Buffer> buffer;

  UniformBuffer() = default;

  UniformBuffer(MemoryAllocator& allocator, T object, size_t copies)
      : prototype(std::move(object)),
        buffer(allocator.CreateHostVisibleBuffer(
            vk::BufferUsageFlagBits::eUniformBuffer,
            sizeof(T) * copies)),
        copies(copies),
        current_index(copies - 1u) {
    P_ASSERT(copies > 0u);
  };

  operator bool() const { return static_cast<bool>(buffer); }

  T* operator->() { return &prototype; };

 private:
  size_t copies = 0;
  size_t current_index = 0;
};

}  // namespace pixel
