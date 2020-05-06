#include "command_buffer.h"

#include "logging.h"

namespace pixel {

CommandBuffer::CommandBuffer(vk::Device device,
                             std::shared_ptr<const CommandPool> pool,
                             vk::UniqueCommandBuffer buffer)
    : device_(std::move(device)),
      pool_(pool),
      command_buffer_(std::move(buffer)) {
  P_ASSERT(device_ && pool && command_buffer_);
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

bool CommandBuffer::SubmitWithCompletionCallback(
    vk::ArrayProxy<vk::Semaphore> wait_semaphores,
    vk::ArrayProxy<vk::PipelineStageFlags> wait_stages,
    vk::ArrayProxy<vk::Semaphore> signal_semaphores,
    std::function<void(void)> on_done) {
  if (!on_done) {
    return Submit(std::move(wait_semaphores),    //
                  std::move(wait_stages),        //
                  std::move(signal_semaphores),  //
                  nullptr                        //
    );
  }

  auto pool = pool_.lock();

  if (!pool) {
    P_ERROR << "Pool from which the command buffer was allocated has died.";
    return false;
  }

  auto on_done_fence_result = device_.createFenceUnique({});

  if (on_done_fence_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not allocate on one fence.";
    return false;
  }
  auto on_done_fence = std::move(on_done_fence_result.value);

  if (!Submit(std::move(wait_semaphores), std::move(wait_stages),
              std::move(signal_semaphores), on_done_fence.get())) {
    P_ERROR << "Command buffer submission error.";
    return false;
  }

  // This is going to be moved into a capture.
  auto raw_fence = on_done_fence.get();

  auto fence_release = [fence = std::move(on_done_fence)]() mutable {
    fence.reset();
  };

  if (!pool->GetFenceWaiter().AddCompletionHandler(raw_fence, fence_release)) {
    // Manually release the fence so we don't leak it.
    fence_release();
    P_ERROR << "Could not register a fence completion callback.";
    return false;
  }

  return true;
}

}  // namespace pixel
