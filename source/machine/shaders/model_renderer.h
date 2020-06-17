#pragma once

#include <map>
#include <vector>

#include "glm.h"
#include "vulkan.h"

namespace pixel {
namespace shaders {
namespace model_renderer {

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texture_coords;

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
    return {
        // Position
        {

            0u,                                        // location
            0u,                                        // binding
            ToVKFormat<decltype(Vertex::position)>(),  // format
            offsetof(Vertex, position)                 // offset
        },
        // Normal
        {

            1u,                                      // location
            0u,                                      // binding
            ToVKFormat<decltype(Vertex::normal)>(),  // format
            offsetof(Vertex, normal)                 // offset
        },
        // Texture Coords
        {

            2u,                                              // location
            0u,                                              // binding
            ToVKFormat<decltype(Vertex::texture_coords)>(),  // format
            offsetof(Vertex, texture_coords)                 // offset
        },
    };
  }
};

struct DrawOp {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

struct DrawData {
  std::map<vk::PrimitiveTopology, std::vector<DrawOp>> ops;

  void AddDrawOp(vk::PrimitiveTopology topology,
                 std::vector<Vertex> vertices,
                 std::vector<uint32_t> indices) {
    ops[topology].emplace_back(DrawOp{std::move(vertices), std::move(indices)});
  }

  std::vector<vk::PrimitiveTopology> RequiredTopologies() const {
    std::vector<vk::PrimitiveTopology> topologies;
    for (const auto& op : ops) {
      topologies.push_back(op.first);
    }
    return topologies;
  }
};

struct UniformBuffer {
  glm::mat4 mvp;

  UniformBuffer() = default;

  static vk::UniqueDescriptorSetLayout CreateDescriptorSetLayout(
      vk::Device device) {
    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings = {
        // MVP
        {
            0u,                                  // binding
            vk::DescriptorType::eUniformBuffer,  // type
            1u,                                  // descriptor count
            vk::ShaderStageFlagBits::eVertex,    // shader stage
        },
        // Texture Sampler
        {
            1u,                                         // binding
            vk::DescriptorType::eCombinedImageSampler,  // type
            1u,                                         // descriptor count
            vk::ShaderStageFlagBits::eFragment,         // shader stage
        },
    };

    vk::DescriptorSetLayoutCreateInfo layout_create_info = {
        {},                                             // flags
        static_cast<uint32_t>(layout_bindings.size()),  // bindings size
        layout_bindings.data()};

    return UnwrapResult(
        device.createDescriptorSetLayoutUnique(layout_create_info));
  }
};

}  // namespace model_renderer
}  // namespace shaders
}  // namespace pixel
