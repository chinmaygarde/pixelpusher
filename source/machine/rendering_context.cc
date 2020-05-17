#include "rendering_context.h"

#include "logging.h"

namespace pixel {

RenderingContext::RenderingContext(vk::PhysicalDevice physical_device,
                                   vk::Device device,
                                   QueueSelection graphics_queue,
                                   QueueSelection transfer_queue,
                                   vk::RenderPass onscreen_render_pass) {
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

  onscreen_render_pass_ = onscreen_render_pass;
  if (!onscreen_render_pass_) {
    return;
  }

  is_valid_ = true;
}

RenderingContext::~RenderingContext() = default;

bool RenderingContext::IsValid() const {
  return is_valid_;
}

MemoryAllocator& RenderingContext::GetMemoryAllocator() const {
  P_ASSERT(is_valid_);
  return *memory_allocator_;
}

vk::PipelineCache RenderingContext::GetPipelineCache() const {
  P_ASSERT(is_valid_);
  return *pipeline_cache_;
}

const CommandPool& RenderingContext::GetGraphicsCommandPool() const {
  P_ASSERT(is_valid_);
  return *graphics_command_pool_;
}

const CommandPool& RenderingContext::GetTransferCommandPool() const {
  P_ASSERT(is_valid_);
  return *transfer_command_pool_;
}

vk::RenderPass RenderingContext::GetOnScreenRenderPass() const {
  P_ASSERT(is_valid_);
  return onscreen_render_pass_;
}

}  // namespace pixel
