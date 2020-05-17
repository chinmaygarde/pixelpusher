#pragma once

#include "macros.h"
#include "memory_allocator.h"
#include "vulkan.h"

namespace pixel {

class RenderingContext {
 public:
  RenderingContext(vk::PhysicalDevice physical_device,
                   vk::Device logical_device);

  ~RenderingContext();

  bool IsValid() const;

  MemoryAllocator& GetMemoryAllocator() const;

  vk::PipelineCache GetPipelineCache() const;

 private:
  std::unique_ptr<MemoryAllocator> memory_allocator_;
  vk::UniquePipelineCache pipeline_cache_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(RenderingContext);
};

}  // namespace pixel
