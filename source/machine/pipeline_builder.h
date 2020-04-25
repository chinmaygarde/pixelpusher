#pragma once

#include <vector>

#include "macros.h"
#include "vulkan_connection.h"

namespace pixel {

class PipelineBuilder {
 public:
  PipelineBuilder();

  ~PipelineBuilder();

  vk::UniquePipeline CreatePipeline(
      const vk::Device& device,
      const std::vector<vk::PipelineShaderStageCreateInfo>& shader_stages,
      vk::PipelineLayout pipeline_layout,
      vk::RenderPass render_pass) const;

  PipelineBuilder& SetScissor(vk::Rect2D rect);

  PipelineBuilder& SetViewport(vk::Viewport viewport);

 private:
  vk::PipelineVertexInputStateCreateInfo vertex_input_state_;
  vk::PipelineInputAssemblyStateCreateInfo input_assembly_;
  vk::Viewport viewport_;
  vk::Rect2D scissor_;
  vk::PipelineViewportStateCreateInfo viewport_state_;
  vk::PipelineRasterizationStateCreateInfo rasterizer_state_;
  vk::PipelineMultisampleStateCreateInfo multisample_state_;
  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_;
  vk::PipelineColorBlendAttachmentState color_blend_attachment_state_;
  vk::PipelineColorBlendStateCreateInfo color_blend_state_;
  std::vector<vk::DynamicState> dynamic_states_;
  vk::PipelineDynamicStateCreateInfo dynamic_state_;

  P_DISALLOW_COPY_AND_ASSIGN(PipelineBuilder);
};

}  // namespace pixel
