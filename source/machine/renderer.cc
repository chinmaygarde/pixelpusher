#include "renderer.h"

#include <vector>

#include "command_buffer.h"
#include "glm.h"
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

  const std::vector<TriangleVertices> vertices = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  const std::vector<uint16_t> indices = {2, 1, 0, 0, 3, 2};

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

  UniformBuffer<TriangleUBO> triangle_ubo(
      connection_.GetMemoryAllocator(), {},
      connection_.GetSwapchain().GetImageCount());

  if (!triangle_ubo) {
    P_ERROR << "Could not allocate backing store for the triangle UBO.";
    return false;
  }

  triangle_ubo_ = std::move(triangle_ubo);

  auto descriptor_set_layout =
      TriangleUBO::CreateDescriptorSetLayout(connection_.GetDevice());

  if (!descriptor_set_layout) {
    P_ERROR << "Could not create descriptor set layout.";
    return false;
  }

  descriptor_set_layout_ = std::move(descriptor_set_layout);

  vk::DescriptorPoolSize pool_size;
  pool_size.setDescriptorCount(connection_.GetSwapchain().GetImageCount());
  pool_size.setType(vk::DescriptorType::eUniformBuffer);

  vk::DescriptorPoolCreateInfo descriptor_pool_info;
  descriptor_pool_info.setPoolSizeCount(1u);
  descriptor_pool_info.setPPoolSizes(&pool_size);
  descriptor_pool_info.setMaxSets(connection_.GetSwapchain().GetImageCount());

  auto descriptor_pool = UnwrapResult(
      connection_.GetDevice().createDescriptorPoolUnique(descriptor_pool_info));
  if (!descriptor_pool) {
    P_ERROR << "Could not create descriptor pool.";
    return false;
  }

  vk::DescriptorSetAllocateInfo descriptor_info;
  descriptor_info.setDescriptorPool(*descriptor_pool);
  descriptor_info.setDescriptorSetCount(
      connection_.GetSwapchain().GetImageCount());
  auto layouts = std::vector<vk::DescriptorSetLayout>{
      connection_.GetSwapchain().GetImageCount(), descriptor_set_layout_.get()};
  descriptor_info.setPSetLayouts(layouts.data());

  descriptor_sets_ = UnwrapResult(
      connection_.GetDevice().allocateDescriptorSets(descriptor_info));

  if (descriptor_sets_.size() != connection_.GetSwapchain().GetImageCount()) {
    P_ERROR << "Could not allocate descriptor sets.";
    return false;
  }

  descriptor_pool_ = std::move(descriptor_pool);

  std::vector<vk::DescriptorSetLayout> layouts2;
  layouts2.push_back(descriptor_set_layout_.get());

  // Setup pipeline layout.
  pipeline_layout_ = CreatePipelineLayout(connection_.GetDevice(), layouts2);

  PipelineBuilder pipeline_builder;

  const auto extents = connection_.GetSwapchain().GetExtents();

  triangle_ubo_->model = glm::identity<glm::mat4>();
  triangle_ubo_->view =
      glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));
  triangle_ubo_->projection = glm::perspective(
      glm::radians(45.0f), static_cast<float>(extents.width) / extents.height,
      0.1f, 10.0f);

  pipeline_builder.SetScissor({{0u, 0u}, {extents.width, extents.height}});
  pipeline_builder.SetViewport({0.0f, 0.0f, static_cast<float>(extents.width),
                                static_cast<float>(extents.height), 0.0f,
                                1.0f});
  pipeline_builder.SetVertexInputDescription(
      {TriangleVertices::GetVertexInputBindingDescription()},
      TriangleVertices::GetVertexInputAttributeDescription());

  auto pipeline = pipeline_builder.CreatePipeline(
      connection_.GetDevice(),                    // device
      shader_stages,                              // shader stages
      pipeline_layout_.get(),                     // pipeline layout
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

  if (!triangle_ubo_.UpdateUniformData()) {
    P_ERROR << "Could not write uniform data.";
    return false;
  }

  const auto current_index = triangle_ubo_.GetCurrentIndex();

  std::vector<vk::WriteDescriptorSet> write_descriptor_sets;

  auto buffer_info = triangle_ubo_.GetBufferInfo();
  vk::WriteDescriptorSet write_descriptor_set;
  write_descriptor_set.setDescriptorCount(1u);
  write_descriptor_set.setDescriptorType(vk::DescriptorType::eUniformBuffer);
  write_descriptor_set.setDstArrayElement(0u);
  write_descriptor_set.setDstSet(descriptor_sets_[current_index]);
  write_descriptor_set.setPBufferInfo(&buffer_info);
  write_descriptor_sets.push_back(write_descriptor_set);

  connection_.GetDevice().updateDescriptorSets(write_descriptor_sets, nullptr);

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
  buffer.value().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                    pipeline_layout_.get(), 0u,
                                    descriptor_sets_[current_index], nullptr);
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
