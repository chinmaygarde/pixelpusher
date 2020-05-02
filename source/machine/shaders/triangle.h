#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "vulkan.h"

namespace pixel {

struct Triangle {
  glm::vec2 position;
  glm::vec3 color;

  static vk::VertexInputBindingDescription GetVertexInputBindingDescription() {
    vk::VertexInputBindingDescription desc;
    desc.setBinding(0u);
    desc.setInputRate(vk::VertexInputRate::eVertex);
    desc.setStride(sizeof(Triangle));
    return desc;
  };

  static std::vector<vk::VertexInputAttributeDescription>
  GetVertexInputAttributeDescription() {
    std::vector<vk::VertexInputAttributeDescription> desc;

    desc.resize(2u);

    // Position.
    desc[0].setBinding(0u);
    desc[0].setOffset(offsetof(Triangle, position));
    desc[0].setLocation(0u);
    desc[0].setFormat(vk::Format::eR32G32Sfloat);

    // Color.
    desc[1].setBinding(0u);
    desc[1].setOffset(offsetof(Triangle, color));
    desc[1].setLocation(1u);
    desc[1].setFormat(vk::Format::eR32G32B32Sfloat);

    return desc;
  }
};

}  // namespace pixel
