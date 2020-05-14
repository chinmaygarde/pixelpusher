#pragma once

#include <functional>

#include "command_pool.h"
#include "macros.h"
#include "vulkan.h"

namespace pixel {

class CommandBuffer {
 public:
  bool Submit(vk::ArrayProxy<vk::Semaphore> wait_semaphores = nullptr,
              vk::ArrayProxy<vk::PipelineStageFlags> wait_stages = nullptr,
              vk::ArrayProxy<vk::Semaphore> signal_semaphores = nullptr,
              vk::Fence submission_fence = nullptr);

  bool SubmitWithCompletionCallback(
      vk::ArrayProxy<vk::Semaphore> wait_semaphores = nullptr,
      vk::ArrayProxy<vk::PipelineStageFlags> wait_stages = nullptr,
      vk::ArrayProxy<vk::Semaphore> signal_semaphores = nullptr,
      std::function<void(void)> on_done = nullptr);

  ~CommandBuffer();

  const vk::CommandBuffer& GetCommandBuffer() const;

 private:
  friend class CommandPool;

  const vk::Device device_;
  // TODO: This does not need to be weak.
  std::weak_ptr<const CommandPool> pool_;
  vk::UniqueCommandBuffer command_buffer_;

  CommandBuffer(vk::Device device,
                std::shared_ptr<const CommandPool> pool,
                vk::UniqueCommandBuffer buffer);

  P_DISALLOW_COPY_AND_ASSIGN(CommandBuffer);
};

}  // namespace pixel
