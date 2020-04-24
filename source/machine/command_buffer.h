#pragma once

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class ScopedCommandBuffer {
 public:
  ScopedCommandBuffer(const vk::CommandBuffer& buffer);

  ~ScopedCommandBuffer();

  bool IsValid() const;

 private:
  const vk::CommandBuffer& buffer_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(ScopedCommandBuffer);
};

}  // namespace pixel
