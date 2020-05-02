#include "memory_allocator.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace pixel {

static VmaVulkanFunctions CreateVulkanFunctionsProcTable() {
  VmaVulkanFunctions functions = {};

#define BIND_VMA_PROC(proc_name) \
  functions.##proc_name = VULKAN_HPP_DEFAULT_DISPATCHER.##proc_name;

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
                                 const vk::Device& logical_device) {
  proc_table_ = CreateVulkanFunctionsProcTable();
  VmaAllocatorCreateInfo create_info = {};
  create_info.physicalDevice = physical_device;
  create_info.device = logical_device;
  create_info.pVulkanFunctions = &proc_table_;

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

BufferAllocation MemoryAllocator::CreateBuffer(
    const vk::BufferCreateInfo& buffer_info,
    const VmaAllocationCreateInfo& allocation_info) {
  VkBuffer buffer = nullptr;
  VmaAllocation allocation = nullptr;
  if (::vmaCreateBuffer(allocator_,                                     //
                        &static_cast<VkBufferCreateInfo>(buffer_info),  //
                        &allocation_info,                               //
                        &buffer,                                        //
                        &allocation,                                    //
                        nullptr  // allocation info
                        ) != VK_SUCCESS) {
    return {};
  }

  // TODO: Wire up the deleter.
  auto unique_buffer =
      vk::UniqueBuffer{buffer, vk::UniqueBuffer::ObjectDestroy()};

  return {std::move(unique_buffer), allocator_, allocation};
}

bool MemoryAllocator::IsValid() const {
  return is_valid_;
}

}  // namespace pixel
