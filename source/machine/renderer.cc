#include "renderer.h"

#include <vector>

#include "logging.h"
#include "pipeline_builder.h"
#include "pipeline_layout.h"
#include "shader_loader.h"
#include "vulkan_swapchain.h"

namespace pixel {

Renderer::Renderer(GLFWwindow* window) : connection_(window) {
  if (!connection_.IsValid()) {
    P_ERROR << "Vulkan connection was invalid.";
    return;
  }

  is_valid_ = true;
}

Renderer::~Renderer() = default;

bool Renderer::IsValid() const {
  return is_valid_;
}

bool Renderer::Setup() {
  if (!is_valid_) {
    return false;
  }

  // Load shader stages.
  auto vertex_shader_module =
      LoadShaderModule(connection_.GetDevice(), "triangle.vert");
  auto fragment_shader_module =
      LoadShaderModule(connection_.GetDevice(), "triangle.frag");

  if (!vertex_shader_module || !fragment_shader_module) {
    P_ERROR << "Could not load shader modules.";
    return false;
  }

  vk::PipelineShaderStageCreateInfo vertex_shader;
  vertex_shader.setModule(vertex_shader_module.get());
  vertex_shader.setStage(vk::ShaderStageFlagBits::eVertex);
  vertex_shader.setPName("main");

  vk::PipelineShaderStageCreateInfo fragment_shader;
  fragment_shader.setModule(fragment_shader_module.get());
  fragment_shader.setStage(vk::ShaderStageFlagBits::eFragment);
  fragment_shader.setPName("main");

  std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = {
      vertex_shader,    //
      fragment_shader,  //
  };

  // Setup pipeline layout.
  auto pipeline_layout = CreatePipelineLayout(connection_.GetDevice());

  PipelineBuilder pipeline_builder;
  auto pipeline = pipeline_builder.CreatePipeline(
      connection_.GetDevice(),                    // device
      shader_stages,                              // shader stages
      pipeline_layout.get(),                      // pipeline layout
      connection_.GetSwapchain().GetRenderPass()  // render pass
  );

  if (!pipeline) {
    P_ERROR << "Could not create pipeline.";
    return false;
  }

  pipeline_ = std::move(pipeline);

  return true;
}

bool Renderer::Render() {
  if (!is_valid_) {
    P_ERROR << "Render was not valid.";
    return false;
  }

  auto buffer = connection_.GetSwapchain().AcquireNextCommandBuffer();
  if (!buffer.has_value()) {
    P_ERROR << "Could not acquire next swapchain command buffer.";
    return false;
  }

  // Perform per frame rendering operations here.

  buffer.value().bindPipeline(vk::PipelineBindPoint::eGraphics,
                              pipeline_.get());
  buffer.value().draw(3u,  // vertex count
                      1u,  // instance count
                      0u,  // first vertex
                      0u   // first instance
  );

  if (!connection_.GetSwapchain().SubmitCommandBuffer(buffer.value())) {
    P_ERROR << "Could not submit the command buffer back to the swapchain.";
    return false;
  }

  return true;
}

bool Renderer::Teardown() {
  return true;
}

}  // namespace pixel
