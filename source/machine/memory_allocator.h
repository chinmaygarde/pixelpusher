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

class MemoryAllocator {
 public:
  MemoryAllocator(const vk::PhysicalDevice& physical_device,
                  const vk::UniqueDevice& logical_device);

  ~MemoryAllocator();

  bool IsValid() const;

  std::unique_ptr<Buffer> CreateBuffer(
      const vk::BufferCreateInfo& buffer_info,
      const VmaAllocationCreateInfo& allocation_info);

 private:
  const vk::UniqueDevice& device_;
  VmaVulkanFunctions proc_table_;
  VmaAllocator allocator_ = nullptr;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(MemoryAllocator);
};

}  // namespace pixel
