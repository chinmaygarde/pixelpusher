#include "renderer.h"

#include <vector>

#include "logging.h"
#include "pipeline_builder.h"
#include "render_pass.h"
#include "shader_loader.h"

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

  // Load shaders.
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
  vertex_shader.setPName("main");

  std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = {
      vertex_shader,    //
      fragment_shader,  //
  };

  // Setup render pass.
  auto render_pass = CreateRenderPass(connection_.GetDevice(),
                                      connection_.GetColorAttachmentFormat());
  if (!render_pass) {
    P_ERROR << "Could not create render pass";
    return false;
  }

  PipelineBuilder pipeline_builder;
  pipeline_builder.CreatePipeline(connection_.GetDevice(), shader_stages,
                                  render_pass.get());

  return true;
}

bool Renderer::Render() {
  return true;
}

bool Renderer::Teardown() {
  return true;
}

}  // namespace pixel
