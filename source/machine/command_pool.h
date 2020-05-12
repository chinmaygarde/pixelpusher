#pragma once

#include <memory>

#include "fence_waiter.h"
#include "macros.h"
#include "vulkan.h"

namespace pixel {

class CommandBuffer;
class CommandPool : public std::enable_shared_from_this<CommandPool> {
 public:
  static std::shared_ptr<CommandPool> Create(vk::Device device,
                                             vk::CommandPoolCreateFlags flags,
                                             uint32_t queue_family_index);

  ~CommandPool();

  std::shared_ptr<CommandBuffer> CreateCommandBuffer() const;

  vk::CommandPool GetCommandPool() const;

  const vk::Queue& GetCommandQueue() const;

  FenceWaiter& GetFenceWaiter() const;

 private:
  std::shared_ptr<FenceWaiter> waiter_;
  vk::Device device_;
  vk::UniqueCommandPool pool_;
  vk::Queue queue_;

  CommandPool(vk::Device device,
              vk::UniqueCommandPool pool,
              vk::Queue queue,
              std::shared_ptr<FenceWaiter> waiter);

  P_DISALLOW_COPY_AND_ASSIGN(CommandPool);
};

}  // namespace pixel
