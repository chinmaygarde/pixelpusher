#pragma once

#include <memory>

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

  std::unique_ptr<CommandBuffer> CreateCommandBuffer() const;

  const vk::Queue& GetCommandQueue() const;

 private:
  vk::Device device_;
  vk::UniqueCommandPool pool_;
  vk::Queue queue_;

  CommandPool(vk::Device device, vk::UniqueCommandPool pool, vk::Queue queue);

  P_DISALLOW_COPY_AND_ASSIGN(CommandPool);
};

}  // namespace pixel
