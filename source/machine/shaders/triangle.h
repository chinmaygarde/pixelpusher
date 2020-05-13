#pragma once

#include <vector>

#include "glm.h"
#include "vulkan.h"

namespace pixel {

struct TriangleVertices {
  glm::vec2 position;
  glm::vec2 color;

  static vk::VertexInputBindingDescription GetVertexInputBindingDescription() {
    vk::VertexInputBindingDescription desc;
    desc.setBinding(0u);
    desc.setInputRate(vk::VertexInputRate::eVertex);
    desc.setStride(sizeof(TriangleVertices));
    return desc;
  };

  static std::vector<vk::VertexInputAttributeDescription>
  GetVertexInputAttributeDescription() {
    std::vector<vk::VertexInputAttributeDescription> desc;

    desc.resize(2u);

    // Position.
    desc[0].setBinding(0u);
    desc[0].setOffset(offsetof(TriangleVertices, position));
    desc[0].setLocation(0u);
    desc[0].setFormat(vk::Format::eR32G32Sfloat);

    // Texture Coordinate.
    desc[1].setBinding(0u);
    desc[1].setOffset(offsetof(TriangleVertices, color));
    desc[1].setLocation(1u);
    desc[1].setFormat(vk::Format::eR32G32Sfloat);

    return desc;
  }
};

struct TriangleUBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;

  static vk::UniqueDescriptorSetLayout CreateDescriptorSetLayout(
      vk::Device device) {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    {
      vk::DescriptorSetLayoutBinding binding;
      binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
      binding.setBinding(0u);
      binding.setDescriptorCount(1u);
      binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
      bindings.push_back(binding);
    }

    {
      vk::DescriptorSetLayoutBinding binding;
      binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
      binding.setBinding(1u);
      binding.setDescriptorCount(1u);
      binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
      bindings.push_back(binding);
    }

    vk::DescriptorSetLayoutCreateInfo layout_info;
    layout_info.setBindingCount(bindings.size());
    layout_info.setPBindings(bindings.data());

    return UnwrapResult(device.createDescriptorSetLayoutUnique(layout_info));
  }
};

}  // namespace pixel
