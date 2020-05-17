#include "command_pool.h"

#include "command_buffer.h"
#include "logging.h"

namespace pixel {

CommandPool::CommandPool(vk::Device device,
                         vk::UniqueCommandPool pool,
                         vk::Queue queue,
                         std::shared_ptr<FenceWaiter> waiter)
    : waiter_(std::move(waiter)),
      device_(std::move(device)),
      pool_(std::move(pool)),
      queue_(std::move(queue)) {}

CommandPool::~CommandPool() = default;

std::shared_ptr<CommandPool> CommandPool::Create(
    vk::Device device,
    vk::CommandPoolCreateFlags flags,
    uint32_t queue_family_index,
    vk::Queue queue) {
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

  auto fence_waiter = FenceWaiter::Create(device, queue);
  if (!fence_waiter) {
    P_ERROR << "Could not create fence waiter.";
    return nullptr;
  }

  return std::shared_ptr<CommandPool>(new CommandPool(
      std::move(device),        //
      std::move(result.value),  //
      std::move(queue),         //
      std::move(fence_waiter)   //
      ));
}

std::shared_ptr<CommandBuffer> CommandPool::CreateCommandBuffer() const {
  vk::CommandBufferAllocateInfo buffer_info;
  buffer_info.setCommandPool(pool_.get());
  buffer_info.setLevel(vk::CommandBufferLevel::ePrimary);
  buffer_info.setCommandBufferCount(1u);

  auto result = device_.allocateCommandBuffersUnique(buffer_info);

  if (result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not allocate command buffer.";
    return nullptr;
  }
  return std::shared_ptr<CommandBuffer>(new CommandBuffer(
      device_, shared_from_this(), std::move(result.value.front())));
}

vk::Queue CommandPool::GetCommandQueue() const {
  return queue_;
}

vk::CommandPool CommandPool::GetCommandPool() const {
  return *pool_;
}

FenceWaiter& CommandPool::GetFenceWaiter() const {
  return *waiter_;
}

vk::Device CommandPool::GetDevice() const {
  return device_;
}

}  // namespace pixel
