#include "rendering_context.h"

#include "logging.h"

namespace pixel {

RenderingContext::RenderingContext(vk::PhysicalDevice physical_device,
                                   vk::Device device) {
  memory_allocator_ =
      std::make_unique<MemoryAllocator>(physical_device, device);
  if (!memory_allocator_->IsValid()) {
    return;
  }

  pipeline_cache_ = UnwrapResult(device.createPipelineCacheUnique({}));
  if (!pipeline_cache_) {
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

}  // namespace pixel
