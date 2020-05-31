#pragma once

#include <vk_mem_alloc.h>

#include <memory>
#include <vector>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

struct Buffer {
  vk::Buffer buffer = {};
  VmaAllocator allocator = nullptr;
  VmaAllocation allocation = nullptr;
  VmaAllocationInfo allocation_info = {};

  Buffer(vk::Buffer buffer,
         VmaAllocator allocator,
         VmaAllocation allocation,
         VmaAllocationInfo allocation_info);

  ~Buffer();

  size_t GetSize() const;

  P_DISALLOW_COPY_AND_ASSIGN(Buffer);
};

struct Image {
  vk::Image image = {};
  VmaAllocator allocator = nullptr;
  VmaAllocation allocation = nullptr;

  Image(vk::Image image, VmaAllocator allocator, VmaAllocation allocation);

  ~Image();
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

  std::unique_ptr<Image> CreateImage(
      const vk::ImageCreateInfo& image_info,
      const VmaAllocationCreateInfo& allocation_info);

  std::unique_ptr<Buffer> CreateHostVisibleBufferCopy(
      vk::BufferUsageFlags usage,
      const void* buffer,
      size_t buffer_size);

  std::unique_ptr<Buffer> CreateHostVisibleBuffer(vk::BufferUsageFlags usage,
                                                  size_t buffer_size);

  std::unique_ptr<Buffer> CreateDeviceLocalBuffer(vk::BufferUsageFlags usage,
                                                  size_t buffer_size);

  template <class U>
  std::unique_ptr<Buffer> CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlags usage,
      const std::vector<U>& buffer,
      const CommandPool& pool,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done) {
    return CreateDeviceLocalBufferCopy(
        usage, buffer.data(), buffer.size() * sizeof(U), pool, wait_semaphores,
        wait_stages, signal_semaphores, on_done);
  }

  std::unique_ptr<Buffer> CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlags usage,
      const void* buffer,
      size_t buffer_size,
      const CommandPool& pool,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done);

  std::unique_ptr<Image> CreateDeviceLocalImageCopy(
      vk::ImageCreateInfo image_info,
      const void* image_data,
      size_t image_data_size,
      const CommandPool& pool,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done);

  void TraceUsageStatistics() const;

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
