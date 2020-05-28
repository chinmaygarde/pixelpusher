#include "rendering_context.h"

#include "logging.h"

namespace pixel {

RenderingContext::RenderingContext(const Delegate& delegate,
                                   vk::Instance instance,
                                   vk::PhysicalDevice physical_device,
                                   vk::Device logical_device,
                                   QueueSelection graphics_queue,
                                   QueueSelection transfer_queue,
                                   const vk::PhysicalDeviceFeatures& features)
    : delegate_(delegate),
      instance_(instance),
      physical_device_(physical_device),
      device_(logical_device),
      graphics_queue_(graphics_queue),
      transfer_queue_(transfer_queue),
      features_(features) {
  if (!device_) {
    return;
  }

  memory_allocator_ =
      std::make_unique<MemoryAllocator>(physical_device, device_);
  if (!memory_allocator_->IsValid()) {
    return;
  }

  pipeline_cache_ = UnwrapResult(device_.createPipelineCacheUnique({}));
  if (!pipeline_cache_) {
    return;
  }

  graphics_command_pool_ = CommandPool::Create(
      device_, vk::CommandPoolCreateFlagBits::eTransient,
      graphics_queue.queue_family_index, graphics_queue.queue);
  if (!graphics_command_pool_) {
    return;
  }

  transfer_command_pool_ = CommandPool::Create(
      device_, vk::CommandPoolCreateFlagBits::eTransient,
      transfer_queue.queue_family_index, transfer_queue.queue);
  if (!transfer_command_pool_) {
    return;
  }

  descriptor_pool_ = std::make_unique<DescriptorPool>(device_);
  if (!descriptor_pool_) {
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
  return delegate_.GetOnScreenRenderPass();
}

DescriptorPool& RenderingContext::GetDescriptorPool() const {
  return *descriptor_pool_;
}

size_t RenderingContext::GetSwapchainImageCount() const {
  return delegate_.GetSwapchainImageCount();
}

const vk::PhysicalDeviceFeatures& RenderingContext::GetFeatures() const {
  return features_;
}

vk::Extent2D RenderingContext::GetExtents() const {
  return delegate_.GetScreenExtents();
}

vk::Viewport RenderingContext::GetViewport() const {
  const auto extents = GetExtents();
  return {0,
          0,
          static_cast<float>(extents.width),
          static_cast<float>(extents.height),
          0.0,
          1.0};
}

vk::Rect2D RenderingContext::GetScissorRect() const {
  const auto extents = GetExtents();
  return {{0, 0}, {extents.width, extents.height}};
}

}  // namespace pixel
