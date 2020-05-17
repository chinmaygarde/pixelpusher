#pragma once

#include "command_pool.h"
#include "macros.h"
#include "memory_allocator.h"
#include "queue_selection.h"
#include "vulkan.h"

namespace pixel {

class RenderingContext {
 public:
  RenderingContext(vk::PhysicalDevice physical_device,
                   vk::Device logical_device,
                   QueueSelection graphics_queue,
                   QueueSelection transfer_queue);

  ~RenderingContext();

  bool IsValid() const;

  MemoryAllocator& GetMemoryAllocator() const;

  vk::PipelineCache GetPipelineCache() const;

  const CommandPool& GetGraphicsCommandPool() const;

  const CommandPool& GetTransferCommandPool() const;

 private:
  std::unique_ptr<MemoryAllocator> memory_allocator_;
  vk::UniquePipelineCache pipeline_cache_;
  std::shared_ptr<CommandPool> graphics_command_pool_;
  std::shared_ptr<CommandPool> transfer_command_pool_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(RenderingContext);
};

}  // namespace pixel
