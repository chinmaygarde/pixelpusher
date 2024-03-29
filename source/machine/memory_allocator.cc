#include "memory_allocator.h"

#include "platform.h"

GCC_PRAGMA("GCC diagnostic push")
GCC_PRAGMA("GCC diagnostic ignored \"-Wunused-variable\"")
GCC_PRAGMA("GCC diagnostic ignored \"-Wunused-function\"")

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

GCC_PRAGMA("GCC diagnostic pop")

#include <imgui.h>

#include "closure.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "logging.h"
#include "string_utils.h"

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
               VmaAllocation p_allocation,
               VmaAllocationInfo p_allocation_info)
    : buffer(p_buffer),
      allocator(p_allocator),
      allocation(p_allocation),
      allocation_info(p_allocation_info) {}

Buffer::~Buffer() {
  vmaDestroyBuffer(allocator, buffer, allocation);
}

size_t Buffer::GetSize() const {
  return allocation_info.size;
}

Image::Image(vk::Image p_image,
             VmaAllocator p_allocator,
             VmaAllocation p_allocation)
    : image(p_image), allocator(p_allocator), allocation(p_allocation) {}

Image::~Image() {
  vmaDestroyImage(allocator, image, allocation);
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
    const VmaAllocationCreateInfo& allocation_info,
    const char* debug_name) {
  if (buffer_info.size == 0u) {
    P_ERROR << "Cannot create zero sized buffer.";
    return nullptr;
  }

  VkBuffer raw_buffer = nullptr;
  VmaAllocation allocation = nullptr;
  VkBufferCreateInfo buffer_create_info = buffer_info;
  VmaAllocationInfo base_allocation_info = {};
  if (::vmaCreateBuffer(allocator_,            //
                        &buffer_create_info,   //
                        &allocation_info,      //
                        &raw_buffer,           //
                        &allocation,           //
                        &base_allocation_info  // allocation info
                        ) != VK_SUCCESS) {
    return nullptr;
  }

  SetDebugNameF(device_, vk::Buffer{raw_buffer}, "%s Buffer", debug_name);

  return std::make_unique<Buffer>(raw_buffer, allocator_, allocation,
                                  base_allocation_info);
}

std::unique_ptr<Image> MemoryAllocator::CreateImage(
    const vk::ImageCreateInfo& image_info,
    const VmaAllocationCreateInfo& allocation_info,
    const char* debug_name) {
  VkImage raw_image = nullptr;
  VmaAllocation allocation = nullptr;
  VkImageCreateInfo image_create_info = image_info;
  if (::vmaCreateImage(allocator_,          //
                       &image_create_info,  //
                       &allocation_info,    //
                       &raw_image,          //
                       &allocation,         //
                       nullptr              // allocation info
                       ) != VK_SUCCESS) {
    return nullptr;
  }

  SetDebugNameF(device_, vk::Image{raw_image}, "%s Image", debug_name);

  return std::make_unique<Image>(raw_image, allocator_, allocation);
}

bool MemoryAllocator::IsValid() const {
  return is_valid_;
}

std::unique_ptr<Buffer> MemoryAllocator::CreateHostVisibleBufferCopy(
    vk::BufferUsageFlags usage,
    const void* buffer,
    size_t buffer_size,
    const char* debug_name) {
  auto host_visible_buffer = CreateHostVisibleBuffer(
      usage, buffer_size, MakeStringF("%s Host Visible", debug_name).c_str());

  if (!host_visible_buffer) {
    return nullptr;
  }

  BufferMapping mapping(*host_visible_buffer);
  if (!mapping.IsValid()) {
    P_ERROR << "Could not setup host visible buffer mapping.";
    return nullptr;
  }

  memcpy(mapping.GetMapping(), buffer, buffer_size);

  return host_visible_buffer;
}

std::unique_ptr<Buffer> MemoryAllocator::CreateHostVisibleBuffer(
    vk::BufferUsageFlags usage,
    size_t buffer_size,
    const char* debug_name) {
  vk::BufferCreateInfo buffer_info;
  buffer_info.setSize(buffer_size);
  buffer_info.setUsage(usage);
  buffer_info.setSharingMode(vk::SharingMode::eExclusive);

  VmaAllocationCreateInfo allocation_info = {};
  allocation_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
  // Coherence is not necessary as we are manually managing the mappings.
  allocation_info.requiredFlags =
      VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

  auto host_visible_buffer =
      CreateBuffer(buffer_info, allocation_info, debug_name);

  if (!host_visible_buffer) {
    P_ERROR << "Could not create host visible buffer.";
    return nullptr;
  }

  return host_visible_buffer;
}

VmaAllocationCreateInfo DefaultDeviceLocalAllocationCreateInfo() {
  VmaAllocationCreateInfo device_allocation_info = {};
  device_allocation_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
  device_allocation_info.requiredFlags =
      VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  return device_allocation_info;
}

std::unique_ptr<Buffer> MemoryAllocator::CreateDeviceLocalBuffer(
    vk::BufferUsageFlags usage,
    size_t buffer_size,
    const char* debug_name) {
  vk::BufferCreateInfo device_buffer_info;
  device_buffer_info.setUsage(usage | vk::BufferUsageFlagBits::eTransferDst);
  device_buffer_info.setSize(buffer_size);
  device_buffer_info.setSharingMode(vk::SharingMode::eExclusive);

  auto device_allocation_info = DefaultDeviceLocalAllocationCreateInfo();

  auto device_buffer =
      CreateBuffer(device_buffer_info, device_allocation_info,
                   MakeStringF("%s Device Local", debug_name).c_str());

  if (!device_buffer) {
    P_ERROR << "Could not create device buffer.";
    return nullptr;
  }

  return device_buffer;
}

std::unique_ptr<Buffer> MemoryAllocator::CreateDeviceLocalBufferCopy(
    vk::BufferUsageFlags usage,
    const void* buffer,
    size_t buffer_size,
    const CommandPool& pool,
    const char* debug_name,
    vk::ArrayProxy<vk::Semaphore> wait_semaphores,
    vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
    vk::ArrayProxy<vk::Semaphore> signal_semaphores,
    std::function<void(void)> on_done) {
  if (buffer_size == 0 || buffer == nullptr) {
    return nullptr;
  }

  auto copy_callback = [buffer, buffer_size](
                           uint8_t* staging_buffer,
                           size_t staging_buffer_size) -> bool {
    if (buffer_size < staging_buffer_size) {
      return false;
    }

    ::memcpy(staging_buffer, buffer, buffer_size);
    return true;
  };
  return CreateDeviceLocalBufferCopy(usage,              //
                                     copy_callback,      //
                                     buffer_size,        //
                                     pool,               //
                                     debug_name,         //
                                     wait_semaphores,    //
                                     wait_stages,        //
                                     signal_semaphores,  //
                                     on_done             //
  );
}

std::unique_ptr<Buffer> MemoryAllocator::CreateDeviceLocalBufferCopy(
    vk::BufferUsageFlags usage,
    std::function<bool(uint8_t* buffer, size_t buffer_size)> copy_callback,
    size_t buffer_size,
    const CommandPool& pool,
    const char* debug_name,
    vk::ArrayProxy<vk::Semaphore> wait_semaphores,
    vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
    vk::ArrayProxy<vk::Semaphore> signal_semaphores,
    std::function<void(void)> on_done) {
  if (!IsValid()) {
    return nullptr;
  }

  if (buffer_size == 0 || !copy_callback) {
    return nullptr;
  }

  auto staging_buffer = CreateHostVisibleBuffer(
      usage | vk::BufferUsageFlagBits::eTransferSrc,      //
      buffer_size,                                        //
      MakeStringF("%s Host Staging", debug_name).c_str()  //
  );

  if (!staging_buffer) {
    return nullptr;
  }

  {
    BufferMapping mapping(*staging_buffer);
    if (!mapping) {
      return nullptr;
    }

    if (!copy_callback(reinterpret_cast<uint8_t*>(mapping.GetMapping()),
                       buffer_size)) {
      return nullptr;
    }
  }

  // Copy the staging buffer to the device.
  auto device_buffer = CreateDeviceLocalBuffer(usage, buffer_size, debug_name);

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

  { cmd_buffer.end(); }

  auto on_done_fence = device_.createFenceUnique({});

  auto on_transfer_done = MakeCopyable(
      [transfer_command_buffer, staging_buffer = std::move(staging_buffer),
       on_done]() mutable {
        transfer_command_buffer.reset();
        staging_buffer.reset();
        if (on_done) {
          on_done();
        }
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

std::unique_ptr<Image> MemoryAllocator::CreateDeviceLocalImageCopy(
    vk::ImageCreateInfo image_info,
    const void* image_data,
    size_t image_data_size,
    const CommandPool& pool,
    const char* debug_name,
    vk::ArrayProxy<vk::Semaphore> wait_semaphores,
    vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
    vk::ArrayProxy<vk::Semaphore> signal_semaphores,
    std::function<void(void)> on_done) {
  if (!IsValid()) {
    return nullptr;
  }

  if (image_data == nullptr || image_data_size == 0) {
    return nullptr;
  }

  auto staging_buffer =
      CreateHostVisibleBufferCopy(vk::BufferUsageFlagBits::eTransferSrc,
                                  image_data, image_data_size, debug_name);

  if (!staging_buffer) {
    return nullptr;
  }

  auto device_allocation_info = DefaultDeviceLocalAllocationCreateInfo();

  image_info.usage |= vk::ImageUsageFlagBits::eTransferDst;

  auto device_image =
      CreateImage(image_info, device_allocation_info, debug_name);

  if (!device_image) {
    return nullptr;
  }

  auto transfer_command_buffer = pool.CreateCommandBuffer();

  if (!transfer_command_buffer) {
    return nullptr;
  }

  auto cmd_buffer = transfer_command_buffer->GetCommandBuffer();

  {
    vk::CommandBufferBeginInfo begin_info;
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd_buffer.begin(begin_info);
  }

  {
    vk::ImageMemoryBarrier image_barrier;
    image_barrier.setImage(device_image->image);
    image_barrier.setOldLayout(vk::ImageLayout::eUndefined);
    image_barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
    image_barrier.setSrcAccessMask({});
    image_barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
    image_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    image_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    vk::ImageSubresourceRange subresource_range;
    subresource_range.setLayerCount(1u);
    subresource_range.setLevelCount(1u);
    subresource_range.setBaseArrayLayer(0u);
    subresource_range.setBaseMipLevel(0u);
    subresource_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
    image_barrier.setSubresourceRange(subresource_range);
    cmd_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                               vk::PipelineStageFlagBits::eTransfer, {},
                               nullptr, nullptr, {image_barrier});
  }

  {
    vk::BufferImageCopy image_copy;
    image_copy.setBufferOffset(0u);
    image_copy.setBufferImageHeight(0u);  // tightly packed
    image_copy.setBufferRowLength(0u);    // tightly packed
    image_copy.setImageOffset(vk::Offset3D{0u, 0u});
    image_copy.setImageExtent(image_info.extent);

    vk::ImageSubresourceLayers image_subresource_layers;
    image_subresource_layers.setBaseArrayLayer(0u);
    image_subresource_layers.setMipLevel(0u);
    image_subresource_layers.setLayerCount(1u);
    image_subresource_layers.setAspectMask(vk::ImageAspectFlagBits::eColor);
    image_copy.setImageSubresource(image_subresource_layers);

    cmd_buffer.copyBufferToImage(
        staging_buffer->buffer,                // buffer
        device_image->image,                   // image
        vk::ImageLayout::eTransferDstOptimal,  // image layout
        {image_copy}                           // image copies
    );
  }

  {
    vk::ImageMemoryBarrier image_barrier;
    image_barrier.setImage(device_image->image);
    image_barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
    image_barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    image_barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
    image_barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    image_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    image_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    vk::ImageSubresourceRange subresource_range;
    subresource_range.setLayerCount(1u);
    subresource_range.setLevelCount(1u);
    subresource_range.setBaseArrayLayer(0u);
    subresource_range.setBaseMipLevel(0u);
    subresource_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
    image_barrier.setSubresourceRange(subresource_range);
    cmd_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                               vk::PipelineStageFlagBits::eFragmentShader, {},
                               nullptr, nullptr, {image_barrier});
  }

  {  //
    cmd_buffer.end();
  }

  auto on_transfer_done = MakeCopyable(
      [transfer_command_buffer, staging_buffer = std::move(staging_buffer),
       on_done]() mutable {
        transfer_command_buffer.reset();
        staging_buffer.reset();
        if (on_done) {
          on_done();
        }
      });

  if (!transfer_command_buffer->SubmitWithCompletionCallback(
          wait_semaphores, wait_stages, signal_semaphores, on_transfer_done)) {
    return nullptr;
  }

  return device_image;
}

void MemoryAllocator::TraceUsageStatistics() const {
  if (!::ImGui::BeginTabItem("Memory")) {
    return;
  }
  VmaStats stats = {};
  ::vmaCalculateStats(allocator_, &stats);
  ::ImGui::Text("Device Memory Blocks: %u", stats.total.blockCount);
  ::ImGui::Text("Allocations: %u", stats.total.allocationCount);
  ::ImGui::Text("Used MBytes: %.2f", stats.total.usedBytes / 1e6);
  ::ImGui::Text("Unused MBytes: %.2f", stats.total.unusedBytes / 1e6);
  ::ImGui::Text("Unused Ranges: %u", stats.total.unusedRangeCount);
  ::ImGui::EndTabItem();
}

}  // namespace pixel
