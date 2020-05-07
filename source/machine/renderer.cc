#include "renderer.h"

#include <vector>

#include "command_buffer.h"
#include "logging.h"
#include "pipeline_builder.h"
#include "pipeline_layout.h"
#include "shader_loader.h"
#include "shaders/triangle.h"
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

  command_pool_ = CommandPool::Create(
      connection_.GetDevice(), vk::CommandPoolCreateFlagBits::eTransient,
      connection_.GetGraphicsQueueFamilyIndex());

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

  const std::vector<Triangle> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                          {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                          {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                          {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

  auto vertex_buffer =
      connection_.GetMemoryAllocator().CreateDeviceLocalBufferCopy(
          vk::BufferUsageFlagBits::eVertexBuffer,                    // usage
          vertices.data(),                                           // data
          vertices.size() * sizeof(decltype(vertices)::value_type),  // size
          *command_pool_,                                            // pool
          nullptr,  // wait semaphores
          nullptr,  // wait stages
          nullptr,  // signal semaphores
          nullptr   // on done
      );

  auto index_buffer =
      connection_.GetMemoryAllocator().CreateDeviceLocalBufferCopy(
          vk::BufferUsageFlagBits::eIndexBuffer,                   // usage
          indices.data(),                                          // data
          indices.size() * sizeof(decltype(indices)::value_type),  // size
          *command_pool_,                                          // pool
          nullptr,  // wait semaphores
          nullptr,  // wait stages
          nullptr,  // signal semaphores
          nullptr   // on done
      );

  if (!vertex_buffer || !index_buffer) {
    P_ERROR << "Could not allocate either the vertex or index buffers.";
    return false;
  }

  // TODO: For the transfer to the staging buffer to be complete. Get rid of
  // this and use fences instead.
  connection_.GetDevice().waitIdle();

  vertex_buffer_ = std::move(vertex_buffer);
  index_buffer_ = std::move(index_buffer);

  // Setup pipeline layout.
  auto pipeline_layout = CreatePipelineLayout(connection_.GetDevice());

  PipelineBuilder pipeline_builder;

  const auto extents = connection_.GetSwapchain().GetExtents();

  pipeline_builder.SetScissor({{0u, 0u}, {extents.width, extents.height}});
  pipeline_builder.SetViewport({0.0f, 0.0f, static_cast<float>(extents.width),
                                static_cast<float>(extents.height), 0.0f,
                                1.0f});
  pipeline_builder.SetVertexInputDescription(
      {Triangle::GetVertexInputBindingDescription()},
      Triangle::GetVertexInputAttributeDescription());

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
  buffer.value().bindVertexBuffers(0u, {vertex_buffer_->buffer}, {0u});
  buffer.value().bindIndexBuffer(index_buffer_->buffer, 0,
                                 vk::IndexType::eUint16);
  buffer.value().drawIndexed(6, 1, 0, 0, 0);

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
