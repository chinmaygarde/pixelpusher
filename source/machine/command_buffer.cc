#include "command_buffer.h"

#include "logging.h"

namespace pixel {

ScopedCommandBuffer::ScopedCommandBuffer(const vk::CommandBuffer& buffer)
    : buffer_(buffer) {
  vk::CommandBufferBeginInfo begin_info;

  auto result = buffer_.begin(begin_info);
  if (result != vk::Result::eSuccess) {
    P_ERROR << "Could not begin command buffer";
    return;
  }

  is_valid_ = true;
}

ScopedCommandBuffer::~ScopedCommandBuffer() {
  if (!is_valid_) {
    return;
  }

  auto result = buffer_.end();
  if (result != vk::Result::eSuccess) {
    P_ERROR << "Could not end command buffer.";
  }
}

bool ScopedCommandBuffer::IsValid() const {
  return is_valid_;
}

}  // namespace pixel
