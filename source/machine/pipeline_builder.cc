#include "pipeline_builder.h"

#include "logging.h"

namespace pixel {

PipelineBuilder::PipelineBuilder() {
  input_assembly_.setTopology(vk::PrimitiveTopology::eTriangleList);

  viewport_state_.setScissorCount(1u);
  viewport_state_.setPScissors(&scissor_);
  viewport_state_.setViewportCount(1u);
  viewport_state_.setPViewports(&viewport_);

  rasterizer_state_.setPolygonMode(vk::PolygonMode::eFill);
  rasterizer_state_.setLineWidth(1.0f);
  rasterizer_state_.setCullMode(vk::CullModeFlagBits::eBack);
  rasterizer_state_.setFrontFace(vk::FrontFace::eClockwise);

  multisample_state_.setSampleShadingEnable(false);
  multisample_state_.setRasterizationSamples(vk::SampleCountFlagBits::e1);

  // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
  /*
  // src is object color.
  // dst is buffer color, i.e, the color already in the framebuffer.
  if (blendEnable) {
    finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp>
                     (dstColorBlendFactor * oldColor.rgb);
    finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp>
                   (dstAlphaBlendFactor * oldColor.a);
  } else {
      finalColor = newColor;
  }
  finalColor = finalColor & colorWriteMask;
  */
  color_blend_attachment_state_.setColorWriteMask(
      vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB);
  // Enabled.
  color_blend_attachment_state_.setBlendEnable(false);
  // Color.
  color_blend_attachment_state_.setSrcColorBlendFactor(
      vk::BlendFactor::eSrcAlpha);
  color_blend_attachment_state_.setDstColorBlendFactor(
      vk::BlendFactor::eOneMinusSrcAlpha);
  // Op.
  color_blend_attachment_state_.setColorBlendOp(vk::BlendOp::eAdd);
  // Alpha.
  color_blend_attachment_state_.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
  color_blend_attachment_state_.setDstAlphaBlendFactor(vk::BlendFactor::eZero);

  color_blend_state_.setLogicOpEnable(false);
  color_blend_state_.setLogicOp(vk::LogicOp::eCopy);
  color_blend_state_.setAttachmentCount(1u);
  color_blend_state_.setPAttachments(&color_blend_attachment_state_);
  color_blend_state_.setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

  dynamic_states_ = {
      vk::DynamicState::eViewport,
  };
  dynamic_state_.setDynamicStateCount(dynamic_states_.size());
  dynamic_state_.setPDynamicStates(dynamic_states_.data());
};

PipelineBuilder::~PipelineBuilder() = default;

vk::UniquePipeline PipelineBuilder::CreatePipeline(
    const vk::Device& device,
    const std::vector<vk::PipelineShaderStageCreateInfo>& shader_stages,
    vk::PipelineLayout pipeline_layout,
    vk::RenderPass render_pass) const {
  vk::GraphicsPipelineCreateInfo pipeline_info;
  pipeline_info.setStageCount(shader_stages.size());
  pipeline_info.setPStages(shader_stages.data());
  pipeline_info.setPVertexInputState(&vertex_input_state_);
  pipeline_info.setPInputAssemblyState(&input_assembly_);
  pipeline_info.setPViewportState(&viewport_state_);
  pipeline_info.setPRasterizationState(&rasterizer_state_);
  pipeline_info.setPMultisampleState(&multisample_state_);
  pipeline_info.setPDepthStencilState(&depth_stencil_state_);
  pipeline_info.setPColorBlendState(&color_blend_state_);
  pipeline_info.setPDynamicState(&dynamic_state_);
  pipeline_info.setLayout(pipeline_layout);
  pipeline_info.setRenderPass(render_pass);

  
  auto result = device.createGraphicsPipelineUnique(nullptr,  // pipeline cache
                                                    pipeline_info);
  if (result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create graphics pipeline.";
    return {};
  }

  return std::move(result.value);
}

}  // namespace pixel
