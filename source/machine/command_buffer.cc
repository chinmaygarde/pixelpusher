#include "command_buffer.h"

#include "logging.h"

namespace pixel {

CommandBuffer::CommandBuffer(std::shared_ptr<const CommandPool> pool,
                             vk::UniqueCommandBuffer buffer)
    : pool_(pool), command_buffer_(std::move(buffer)) {
  P_ASSERT(pool && command_buffer_);
}

CommandBuffer::~CommandBuffer() = default;

const vk::CommandBuffer& CommandBuffer::GetCommandBuffer() const {
  return *command_buffer_;
}

bool CommandBuffer::Submit(vk::ArrayProxy<vk::Semaphore> wait_semaphores,
                           vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
                           vk::ArrayProxy<vk::Semaphore> signal_semaphores,
                           vk::Fence submission_fence) {
  if (wait_semaphores.size() != wait_stages.size()) {
    P_ERROR << "Semaphore and wait stage counts were inconsistent.";
    return false;
  }

  vk::SubmitInfo submit_info;

  submit_info.setCommandBufferCount(1u);
  submit_info.setPCommandBuffers(&command_buffer_.get());

  if (wait_semaphores.size() > 0) {
    submit_info.setWaitSemaphoreCount(wait_semaphores.size());
    submit_info.setPWaitSemaphores(wait_semaphores.data());
    submit_info.setPWaitDstStageMask(wait_stages.data());
  }

  if (signal_semaphores.size() > 0) {
    submit_info.setSignalSemaphoreCount(signal_semaphores.size());
    submit_info.setPSignalSemaphores(signal_semaphores.data());
  }

  auto pool = pool_.lock();

  if (!pool) {
    P_ERROR << "Command pool has died.";
    return false;
  }

  if (pool->GetCommandQueue().submit(1u, &submit_info, submission_fence) !=
      vk::Result::eSuccess) {
    P_ERROR << "Could not submit command queue.";
    return false;
  }

  return true;
}

}  // namespace pixel
