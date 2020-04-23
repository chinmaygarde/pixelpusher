#pragma once

#include "vulkan.h"

namespace pixel {

vk::UniqueRenderPass CreateRenderPass(const vk::Device& device,
                                      vk::Format color_attachment_format);

}  // namespace pixel
