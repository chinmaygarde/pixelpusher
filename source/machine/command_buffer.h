#pragma once

#include "command_pool.h"
#include "macros.h"
#include "vulkan.h"

namespace pixel {

class CommandBuffer {
 public:
  const vk::CommandBuffer& GetCommandBuffer() const;

  bool Submit(vk::ArrayProxy<vk::Semaphore> wait_semaphores,
              vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
              vk::ArrayProxy<vk::Semaphore> signal_semaphores);

  ~CommandBuffer();

 private:
  friend class CommandPool;

  std::weak_ptr<const CommandPool> pool_;
  vk::UniqueCommandBuffer command_buffer_;

  CommandBuffer(std::shared_ptr<const CommandPool> pool,
                vk::UniqueCommandBuffer buffer);

  P_DISALLOW_COPY_AND_ASSIGN(CommandBuffer);
};

}  // namespace pixel
