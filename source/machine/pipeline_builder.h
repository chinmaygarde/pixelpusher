#pragma once

#include <set>
#include <vector>

#include "macros.h"
#include "vulkan.h"
#include "vulkan_connection.h"

namespace pixel {

class PipelineBuilder {
 public:
  PipelineBuilder();

  ~PipelineBuilder();

  vk::UniquePipeline CreatePipeline(
      vk::Device device,
      vk::PipelineCache cache,
      vk::PipelineLayout pipeline_layout,
      vk::RenderPass render_pass,
      const std::vector<vk::PipelineShaderStageCreateInfo>& shader_stages)
      const;

  PipelineBuilder& AddDynamicState(vk::DynamicState state);

  PipelineBuilder& SetScissor(vk::Rect2D rect);

  PipelineBuilder& SetViewport(vk::Viewport viewport);

  PipelineBuilder& SetVertexInputDescription(
      std::vector<vk::VertexInputBindingDescription> input_bindings,
      std::vector<vk::VertexInputAttributeDescription> input_attributes);

  PipelineBuilder& SetPrimitiveTopology(vk::PrimitiveTopology topology);

  PipelineBuilder& SetFrontFace(vk::FrontFace front_face);

 private:
  std::vector<vk::VertexInputBindingDescription>
      vertex_input_binding_descriptions_;
  std::vector<vk::VertexInputAttributeDescription>
      vertex_input_attribute_descriptions_;
  vk::PipelineInputAssemblyStateCreateInfo input_assembly_;
  vk::Viewport viewport_;
  vk::Rect2D scissor_;
  vk::PipelineViewportStateCreateInfo viewport_state_;
  vk::PipelineRasterizationStateCreateInfo rasterizer_state_;
  vk::PipelineMultisampleStateCreateInfo multisample_state_;
  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_;
  vk::PipelineColorBlendAttachmentState color_blend_attachment_state_;
  vk::PipelineColorBlendStateCreateInfo color_blend_state_;
  std::set<vk::DynamicState> dynamic_states_;

  P_DISALLOW_COPY_AND_ASSIGN(PipelineBuilder);
};

}  // namespace pixel
