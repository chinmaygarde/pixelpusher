#include "memory_allocator.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#pragma GCC diagnostic pop

#include "logging.h"

namespace pixel {

static VmaVulkanFunctions CreateVulkanFunctionsProcTable() {
  VmaVulkanFunctions functions = {};

#define BIND_VMA_PROC(proc_name) \
  functions.proc_name = VULKAN_HPP_DEFAULT_DISPATCHER.proc_name;

  BIND_VMA_PROC(vkGetPhysicalDeviceProperties);
  BIND_VMA_PROC(vkGetPhysicalDeviceMemoryProperties);
  BIND_VMA_PROC(vkAllocateMemory);
  BIND_VMA_PROC(vkFreeMemory);
  BIND_VMA_PROC(vkMapMemory);
  BIND_VMA_PROC(vkUnmapMemory);
  BIND_VMA_PROC(vkFlushMappedMemoryRanges);
  BIND_VMA_PROC(vkInvalidateMappedMemoryRanges);
  BIND_VMA_PROC(vkBindBufferMemory);
  BIND_VMA_PROC(vkBindImageMemory);
  BIND_VMA_PROC(vkGetBufferMemoryRequirements);
  BIND_VMA_PROC(vkGetImageMemoryRequirements);
  BIND_VMA_PROC(vkCreateBuffer);
  BIND_VMA_PROC(vkDestroyBuffer);
  BIND_VMA_PROC(vkCreateImage);
  BIND_VMA_PROC(vkDestroyImage);
  BIND_VMA_PROC(vkCmdCopyBuffer);
  BIND_VMA_PROC(vkGetBufferMemoryRequirements2KHR);
  BIND_VMA_PROC(vkGetImageMemoryRequirements2KHR);
  BIND_VMA_PROC(vkBindBufferMemory2KHR);
  BIND_VMA_PROC(vkBindImageMemory2KHR);
  BIND_VMA_PROC(vkGetPhysicalDeviceMemoryProperties2KHR);

#undef BIND_VMA_PROC

  return functions;
}

MemoryAllocator::MemoryAllocator(const vk::PhysicalDevice& physical_device,
                                 const vk::UniqueDevice& logical_device)
    : device_(logical_device) {
  if (!device_) {
    P_ERROR << "Invalid device for memory allocator.";
    return;
  }

  proc_table_ = CreateVulkanFunctionsProcTable();
  VmaAllocatorCreateInfo create_info = {};
  create_info.physicalDevice = physical_device;
  create_info.device = device_.get();
  create_info.pVulkanFunctions = &proc_table_;
  if (auto allocator = device_.getAllocator()) {
    const vk::AllocationCallbacks callbacks = *allocator;
    const VkAllocationCallbacks& raw_callbacks = callbacks;
    create_info.pAllocationCallbacks = &raw_callbacks;
  }

  VmaAllocator allocator = nullptr;
  if (vmaCreateAllocator(&create_info, &allocator) != VkResult::VK_SUCCESS) {
    return;
  }

  allocator_ = allocator;
  is_valid_ = true;
}

MemoryAllocator::~MemoryAllocator() {
  if (!is_valid_) {
    return;
  }

  vmaDestroyAllocator(allocator_);
}

std::unique_ptr<Buffer> MemoryAllocator::CreateBuffer(
    const vk::BufferCreateInfo& buffer_info,
    const VmaAllocationCreateInfo& allocation_info) {
  VkBuffer raw_buffer = nullptr;
  VmaAllocation allocation = nullptr;
  VkBufferCreateInfo buffer_create_info = buffer_info;
  if (::vmaCreateBuffer(allocator_,           //
                        &buffer_create_info,  //
                        &allocation_info,     //
                        &raw_buffer,          //
                        &allocation,          //
                        nullptr               // allocation info
                        ) != VK_SUCCESS) {
    return nullptr;
  }

  return std::make_unique<Buffer>(raw_buffer, allocator_, allocation);
}

bool MemoryAllocator::IsValid() const {
  return is_valid_;
}

Buffer::Buffer(vk::Buffer p_buffer,
               VmaAllocator p_allocator,
               VmaAllocation p_allocation)
    : buffer(p_buffer), allocator(p_allocator), allocation(p_allocation) {}

Buffer::~Buffer() {
  if (!allocator || !allocation) {
    return;
  }
  vmaDestroyBuffer(allocator, buffer, allocation);
}

}  // namespace pixel
