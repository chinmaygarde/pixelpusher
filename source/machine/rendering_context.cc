#include "rendering_context.h"

#include "logging.h"

namespace pixel {

RenderingContext::RenderingContext(vk::Instance instance,
                                   vk::PhysicalDevice physical_device,
                                   vk::Device device,
                                   QueueSelection graphics_queue,
                                   QueueSelection transfer_queue,
                                   vk::RenderPass onscreen_render_pass,
                                   size_t swapchain_image_count,
                                   const vk::PhysicalDeviceFeatures& features,
                                   vk::Extent2D extents)
    : instance_(instance),
      physical_device_(physical_device),
      device_(device),
      graphics_queue_(graphics_queue),
      transfer_queue_(transfer_queue),
      swapchain_image_count_(swapchain_image_count),
      features_(features),
      extents_(extents) {
  if (!device_) {
    return;
  }

  memory_allocator_ =
      std::make_unique<MemoryAllocator>(physical_device, device);
  if (!memory_allocator_->IsValid()) {
    return;
  }

  pipeline_cache_ = UnwrapResult(device.createPipelineCacheUnique({}));
  if (!pipeline_cache_) {
    return;
  }

  graphics_command_pool_ = CommandPool::Create(
      device, vk::CommandPoolCreateFlagBits::eTransient,
      graphics_queue.queue_family_index, graphics_queue.queue);
  if (!graphics_command_pool_) {
    return;
  }

  transfer_command_pool_ = CommandPool::Create(
      device, vk::CommandPoolCreateFlagBits::eTransient,
      transfer_queue.queue_family_index, transfer_queue.queue);
  if (!transfer_command_pool_) {
    return;
  }

  descriptor_pool_ = std::make_unique<DescriptorPool>(device);
  if (!descriptor_pool_) {
    return;
  }

  onscreen_render_pass_ = onscreen_render_pass;
  if (!onscreen_render_pass_) {
    return;
  }

  is_valid_ = true;
}

RenderingContext::~RenderingContext() = default;

vk::Instance RenderingContext::GetInstance() const {
  return instance_;
}

vk::PhysicalDevice RenderingContext::GetPhysicalDevice() const {
  return physical_device_;
}

vk::Device RenderingContext::GetDevice() const {
  return device_;
}

bool RenderingContext::IsValid() const {
  return is_valid_;
}

QueueSelection RenderingContext::GetGraphicsQueue() const {
  return graphics_queue_;
}

QueueSelection RenderingContext::GetTransferQueue() const {
  return transfer_queue_;
}

MemoryAllocator& RenderingContext::GetMemoryAllocator() const {
  return *memory_allocator_;
}

vk::PipelineCache RenderingContext::GetPipelineCache() const {
  return *pipeline_cache_;
}

const CommandPool& RenderingContext::GetGraphicsCommandPool() const {
  return *graphics_command_pool_;
}

const CommandPool& RenderingContext::GetTransferCommandPool() const {
  return *transfer_command_pool_;
}

vk::RenderPass RenderingContext::GetOnScreenRenderPass() const {
  return onscreen_render_pass_;
}

const DescriptorPool& RenderingContext::GetDescriptorPool() const {
  return *descriptor_pool_;
}

size_t RenderingContext::GetSwapchainImageCount() const {
  return swapchain_image_count_;
}

const vk::PhysicalDeviceFeatures& RenderingContext::GetFeatures() const {
  return features_;
}

const vk::Extent2D& RenderingContext::GetExtents() const {
  return extents_;
}

}  // namespace pixel
