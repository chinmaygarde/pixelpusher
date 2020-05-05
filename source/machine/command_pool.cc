#include "command_pool.h"

#include "command_buffer.h"
#include "logging.h"

namespace pixel {

CommandPool::CommandPool(vk::Device device,
                         vk::UniqueCommandPool pool,
                         vk::Queue queue)
    : device_(std::move(device)),
      pool_(std::move(pool)),
      queue_(std::move(queue)) {}

CommandPool::~CommandPool() = default;

std::shared_ptr<CommandPool> CommandPool::Create(
    vk::Device device,
    vk::CommandPoolCreateFlags flags,
    uint32_t queue_family_index) {
  if (!device) {
    P_ERROR << "Device was invalid.";
    return nullptr;
  }

  vk::CommandPoolCreateInfo pool_info;
  pool_info.setQueueFamilyIndex(queue_family_index);
  pool_info.setFlags(flags);

  auto result = device.createCommandPoolUnique(pool_info);

  if (result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create command pool.";
    return nullptr;
  }

  // TODO: Allow acquisition of a queue with a different index.
  auto queue = device.getQueue(queue_family_index, 0u);

  return std::shared_ptr<CommandPool>(new CommandPool(
      std::move(device), std::move(result.value), std::move(queue)));
}

std::unique_ptr<CommandBuffer> CommandPool::CreateCommandBuffer() const {
  vk::CommandBufferAllocateInfo buffer_info;
  buffer_info.setCommandPool(pool_.get());
  buffer_info.setLevel(vk::CommandBufferLevel::ePrimary);
  buffer_info.setCommandBufferCount(1u);

  auto result = device_.allocateCommandBuffersUnique(buffer_info);

  if (result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not allocate command buffer.";
    return nullptr;
  }

  return std::unique_ptr<CommandBuffer>(
      new CommandBuffer(shared_from_this(), std::move(result.value.front())));
}

const vk::Queue& CommandPool::GetCommandQueue() const {
  return queue_;
}

}  // namespace pixel
