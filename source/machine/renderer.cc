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

  const std::vector<Triangle> vertices = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                          {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                          {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

  // Copy data into the host visible staging buffer.
  vk::BufferCreateInfo buffer_info;
  buffer_info.setUsage(vk::BufferUsageFlagBits::eVertexBuffer |
                       vk::BufferUsageFlagBits::eTransferSrc);
  buffer_info.setSize(vertices.size() * sizeof(decltype(vertices)::value_type));
  buffer_info.setSharingMode(vk::SharingMode::eExclusive);

  VmaAllocationCreateInfo allocation_info = {};
  allocation_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
  allocation_info.requiredFlags =
      VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  auto staging_buffer = connection_.GetMemoryAllocator().CreateBuffer(
      buffer_info, allocation_info);

  if (!staging_buffer) {
    P_ERROR << "Could not create staging buffer.";
    return false;
  }

  BufferMapping mapping(*staging_buffer);
  if (!mapping.IsValid()) {
    P_ERROR << "Could not setup staging buffer mapping.";
    return false;
  }

  memcpy(mapping.GetMapping(), vertices.data(), buffer_info.size);

  // Copy data into device local buffer.
  vk::BufferCreateInfo device_buffer_info;
  device_buffer_info.setUsage(vk::BufferUsageFlagBits::eVertexBuffer |
                              vk::BufferUsageFlagBits::eTransferDst);
  device_buffer_info.setSize(vertices.size() *
                             sizeof(decltype(vertices)::value_type));
  device_buffer_info.setSharingMode(vk::SharingMode::eExclusive);

  VmaAllocationCreateInfo device_allocation_info = {};
  device_allocation_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
  device_allocation_info.requiredFlags =
      VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  auto device_buffer = connection_.GetMemoryAllocator().CreateBuffer(
      device_buffer_info, device_allocation_info);

  if (!device_buffer) {
    P_ERROR << "Could not create device buffer.";
    return false;
  }

  auto transfer_command_buffer = command_pool_->CreateCommandBuffer();
  if (!transfer_command_buffer) {
    P_ERROR << "Could not create transfer buffer.";
    return false;
  }

  {
    vk::CommandBufferBeginInfo begin_info;
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    transfer_command_buffer->GetCommandBuffer().begin(begin_info);
  }

  {
    vk::BufferCopy region;
    region.setSize(vertices.size() * sizeof(decltype(vertices)::value_type));
    transfer_command_buffer->GetCommandBuffer().copyBuffer(
        staging_buffer.get()->buffer, device_buffer.get()->buffer, {region});
  }

  transfer_command_buffer->GetCommandBuffer().end();

  if (!transfer_command_buffer->Submit(nullptr, nullptr, nullptr)) {
    P_ERROR << "Could not commit transfer command buffer.";
  }

  // TODO: For the transfer to the staging buffer to be complete. Get rid of
  // this and use fences instead.
  connection_.GetDevice().waitIdle();

  vertex_buffer_ = std::move(device_buffer);

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
