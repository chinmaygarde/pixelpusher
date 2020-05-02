#pragma once

#include <vk_mem_alloc.h>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class MemoryAllocator;

template <class T>
class Allocation {
 public:
  Allocation() = default;

  ~Allocation() { Reset(); };

  Allocation(const Allocation&) = delete;

  Allocation(Allocation&& o)
      : buffer_(std::move(o.buffer_)),
        allocator_(o.allocator_),
        allocation_(o.allocation_) {
    o.allocator_ = nullptr;
    o.allocation_ = nullptr;
  }

  Allocation& operator=(const Allocation&) = delete;

  Allocation& operator=(Allocation&& o) {
    Reset();
    std::swap(buffer_, o.buffer_);
    std::swap(allocator_, o.allocator_);
    std::swap(allocation_, o.allocation_);
    return *this;
  }

  operator bool() const { return static_cast<bool>(buffer_); }

  void Reset() {
    buffer_.reset();
    if (allocator_ && allocation_) {
      vmaFreeMemory(allocator_, allocation_);
    }
    allocator_ = nullptr;
    allocation_ = nullptr;
  };

 private:
  friend class MemoryAllocator;
  T buffer_;
  VmaAllocator allocator_ = nullptr;
  VmaAllocation allocation_ = nullptr;

  Allocation(T buffer, VmaAllocator allocator, VmaAllocation allocation)
      : buffer_(std::move(buffer)),
        allocator_(allocator),
        allocation_(allocation) {}
};

using BufferAllocation = Allocation<vk::UniqueBuffer>;

class MemoryAllocator {
 public:
  MemoryAllocator(const vk::PhysicalDevice& physical_device,
                  const vk::Device& logical_device);

  ~MemoryAllocator();

  bool IsValid() const;

  BufferAllocation CreateBuffer(const vk::BufferCreateInfo& buffer_info,
                                const VmaAllocationCreateInfo& allocation_info);

 private:
  VmaVulkanFunctions proc_table_;
  VmaAllocator allocator_ = nullptr;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(MemoryAllocator);
};

}  // namespace pixel
