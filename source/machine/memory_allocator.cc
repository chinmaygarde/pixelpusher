#include "memory_allocator.h"

#include "platform.h"

#if !P_OS_WINDOWS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif  // !P_OS_WINDOWS

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#if !P_OS_WINDOWS
#pragma GCC diagnostic pop
#endif  // !P_OS_WINDOWS

#include "closure.h"
#include "command_buffer.h"
#include "command_pool.h"
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

MemoryAllocator::MemoryAllocator(const vk::PhysicalDevice& physical_device,
                                 vk::Device logical_device)
    : device_(std::move(logical_device)) {
  if (!device_) {
    P_ERROR << "Invalid device for memory allocator.";
    return;
  }

  proc_table_ = CreateVulkanFunctionsProcTable();
  VmaAllocatorCreateInfo create_info = {};
  create_info.physicalDevice = physical_device;
  create_info.device = device_;
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

std::unique_ptr<Buffer> MemoryAllocator::CreateHostVisibleBufferCopy(
    vk::BufferUsageFlags usage,
    const void* buffer,
    size_t buffer_size) {
  vk::BufferCreateInfo buffer_info;
  buffer_info.setSize(buffer_size);
  buffer_info.setUsage(usage);
  buffer_info.setSharingMode(vk::SharingMode::eExclusive);

  VmaAllocationCreateInfo allocation_info = {};
  allocation_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
  // Coherence is not necessary as we are manually managing the mappings.
  allocation_info.requiredFlags =
      VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

  auto host_visible_buffer = CreateBuffer(buffer_info, allocation_info);

  if (!host_visible_buffer) {
    P_ERROR << "Could not create host visible buffer.";
    return false;
  }

  BufferMapping mapping(*host_visible_buffer);
  if (!mapping.IsValid()) {
    P_ERROR << "Could not setup host visible buffer mapping.";
    return false;
  }

  memcpy(mapping.GetMapping(), buffer, buffer_info.size);

  return host_visible_buffer;
}

std::unique_ptr<Buffer> MemoryAllocator::CreateDeviceLocalBufferCopy(
    vk::BufferUsageFlags usage,
    const void* buffer,
    size_t buffer_size,
    const CommandPool& pool,
    vk::ArrayProxy<vk::Semaphore> wait_semaphores,
    vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
    vk::ArrayProxy<vk::Semaphore> signal_semaphores,
    std::function<void(void)> on_done) {
  // Create a host visible staging buffer.
  auto staging_buffer = CreateHostVisibleBufferCopy(
      usage | vk::BufferUsageFlagBits::eTransferSrc, buffer, buffer_size);

  if (!staging_buffer) {
    P_ERROR << "Could not create staging buffer.";
    return nullptr;
  }

  // Copy the staging buffer to the device.
  vk::BufferCreateInfo device_buffer_info;
  device_buffer_info.setUsage(usage | vk::BufferUsageFlagBits::eTransferDst);
  device_buffer_info.setSize(buffer_size);
  device_buffer_info.setSharingMode(vk::SharingMode::eExclusive);

  VmaAllocationCreateInfo device_allocation_info = {};
  device_allocation_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
  device_allocation_info.requiredFlags =
      VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  auto device_buffer = CreateBuffer(device_buffer_info, device_allocation_info);

  if (!device_buffer) {
    P_ERROR << "Could not create device buffer.";
    return nullptr;
  }

  auto transfer_command_buffer = pool.CreateCommandBuffer();

  if (!transfer_command_buffer) {
    P_ERROR << "Could not create transfer buffer.";
    return nullptr;
  }

  auto cmd_buffer = transfer_command_buffer->GetCommandBuffer();

  {
    vk::CommandBufferBeginInfo begin_info;
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd_buffer.begin(begin_info);
  }

  {
    vk::BufferCopy region;
    region.setSize(buffer_size);
    cmd_buffer.copyBuffer(staging_buffer.get()->buffer,
                          device_buffer.get()->buffer, {region});
  }

  cmd_buffer.end();

  auto on_done_fence = device_.createFenceUnique({});

  auto on_transfer_done =
      MakeCopyable([transfer_command_buffer,
                    staging_buffer = std::move(staging_buffer)]() mutable {
        // TODO: This is not thread safe as the command buffer must be released
        // on the same thread as it is allocated.
        P_LOG << "Transfer Done.";
        transfer_command_buffer.reset();
        staging_buffer.reset();
      });

  if (!transfer_command_buffer->SubmitWithCompletionCallback(
          std::move(wait_semaphores),    //
          std::move(wait_stages),        //
          std::move(signal_semaphores),  //
          on_transfer_done)              //
  ) {
    P_ERROR << "Could not commit transfer command buffer.";
    return nullptr;
  }

  return device_buffer;
}

}  // namespace pixel
