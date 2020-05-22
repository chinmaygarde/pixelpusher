#pragma once

#include <vector>

#include "glm.h"
#include "vulkan.h"

namespace pixel {
namespace shaders {
namespace model_renderer {

struct Vertex {
  glm::vec3 position;

  static std::vector<vk::VertexInputBindingDescription>
  GetVertexInputBindings() {
    return {{
        0u,                           // binding
        sizeof(Vertex),               // stride
        vk::VertexInputRate::eVertex  // rate
    }};
  }

  static std::vector<vk::VertexInputAttributeDescription>
  GetVertexInputAttributes() {
    return {{
        // Position
        0u,                                        // location
        0u,                                        // binding
        ToVKFormat<decltype(Vertex::position)>(),  // format
        offsetof(Vertex, position)                 // offset
    }};
  }
};

struct UniformBuffer {
  glm::mat4 mvp;

  static vk::UniqueDescriptorSetLayout CreateDescriptorSetLayout(
      vk::Device device) {
    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings = {
        // MVP
        {
            0u,                                  // binding
            vk::DescriptorType::eUniformBuffer,  // type
            1u,                                  // descriptor count
            vk::ShaderStageFlagBits::eVertex,    // shader stage
        }};

    vk::DescriptorSetLayoutCreateInfo layout_create_info = {
        {},                                             // flags
        static_cast<uint32_t>(layout_bindings.size()),  // bindings size
        layout_bindings.data()};

    return UnwrapResult(
        device.createDescriptorSetLayoutUnique(layout_create_info));
  }
}

}  // namespace model_renderer
}  // namespace shaders
}  // namespace pixel
