#pragma once

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class RenderPass {
 public:
  RenderPass(const vk::Device& device, vk::Format color_attachment_format);

  ~RenderPass();

  bool IsValid() const;

 private:
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(RenderPass);
};

}  // namespace pixel
