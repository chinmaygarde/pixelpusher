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

VmaAllocationCreateInfo DefaultDeviceLocalAllocationCreateInfo();

class CommandPool;
class MemoryAllocator {
 public:
  MemoryAllocator(const vk::PhysicalDevice& physical_device,
                  vk::Device logical_device);

  ~MemoryAllocator();

  bool IsValid() const;

  std::unique_ptr<Buffer> CreateBuffer(
      const vk::BufferCreateInfo& buffer_info,
      const VmaAllocationCreateInfo& allocation_info,
      const char* debug_name);

  std::unique_ptr<Image> CreateImage(
      const vk::ImageCreateInfo& image_info,
      const VmaAllocationCreateInfo& allocation_info,
      const char* debug_name);

  std::unique_ptr<Buffer> CreateHostVisibleBufferCopy(
      vk::BufferUsageFlags usage,
      const void* buffer,
      size_t buffer_size,
      const char* debug_name);

  std::unique_ptr<Buffer> CreateHostVisibleBuffer(vk::BufferUsageFlags usage,
                                                  size_t buffer_size,
                                                  const char* debug_name);

  std::unique_ptr<Buffer> CreateDeviceLocalBuffer(vk::BufferUsageFlags usage,
                                                  size_t buffer_size,
                                                  const char* debug_name);

  template <class U>
  std::unique_ptr<Buffer> CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlags usage,
      const std::vector<U>& buffer,
      const CommandPool& pool,
      const char* debug_name,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done) {
    return CreateDeviceLocalBufferCopy(usage,                      //
                                       buffer.data(),              //
                                       buffer.size() * sizeof(U),  //
                                       pool,                       //
                                       debug_name,                 //
                                       wait_semaphores,            //
                                       wait_stages,                //
                                       signal_semaphores,          //
                                       on_done                     //
    );
  }

  std::unique_ptr<Buffer> CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlags usage,
      const void* buffer,
      size_t buffer_size,
      const CommandPool& pool,
      const char* debug_name,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done);

  std::unique_ptr<Buffer> CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlags usage,
      std::function<bool(uint8_t* buffer, size_t buffer_size)> copy_callback,
      size_t buffer_size,
      const CommandPool& pool,
      const char* debug_name,
      vk::ArrayProxy<vk::Semaphore> wait_semaphores,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores,
      std::function<void(void)> on_done);

  std::unique_ptr<Image> CreateDeviceLocalImageCopy(
      vk::ImageCreateInfo image_info,
      const void* image_data,
      size_t image_data_size,
      const CommandPool& pool,
      const char* debug_name,
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

}  // namespace pixel
