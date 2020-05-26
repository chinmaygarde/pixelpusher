#include "model_renderer.h"

#include <algorithm>
#include <future>

#include <imgui.h>

#include "pipeline_layout.h"

namespace pixel {

ModelRenderer::ModelRenderer(std::shared_ptr<RenderingContext> context,
                             std::string model_assets_dir,
                             std::string model_path)
    : Renderer(context) {
  std::promise<std::unique_ptr<model::Model>> model_promise;
  auto model_future = model_promise.get_future();
  AssetLoader::GetGlobal()->LoadAsset(
      model_assets_dir, model_path,
      MakeCopyable([promise = std::move(model_promise)](
                       std::unique_ptr<Asset> asset) mutable {
        if (!asset) {
          promise.set_value(nullptr);
          return;
        }
        promise.set_value(std::make_unique<model::Model>(*asset));
      }));
  model_ = model_future.get();
  is_valid_ = true;
}

ModelRenderer::~ModelRenderer() = default;

// |Renderer|
bool ModelRenderer::IsValid() const {
  return is_valid_;
}

// |Renderer|
bool ModelRenderer::Setup() {
  if (!is_valid_) {
    return false;
  }

  auto draw_data = model_->GetDrawData();

  auto required_topologies = draw_data.RequiredTopologies();

  if (required_topologies.empty()) {
    return true;
  }

  ShaderLibrary library(GetContext().GetDevice());

  if (!library.AddDefaultVertexShader("model_renderer.vert") ||
      !library.AddDefaultFragmentShader("model_renderer.frag")) {
    return false;
  }

  descriptor_set_layout_ =
      shaders::model_renderer::UniformBuffer::CreateDescriptorSetLayout(
          GetContext().GetDevice());

  if (!descriptor_set_layout_) {
    return false;
  }

  descriptor_sets_ = GetContext().GetDescriptorPool().AllocateDescriptorSets(
      descriptor_set_layout_.get(), GetContext().GetSwapchainImageCount());

  if (!descriptor_sets_) {
    return false;
  }

  PipelineLayoutBuilder pipeline_layout_builder;
  pipeline_layout_builder.AddDescriptorSetLayout(descriptor_set_layout_.get());

  pipeline_layout_ =
      pipeline_layout_builder.CreatePipelineLayout(GetContext().GetDevice());

  if (!pipeline_layout_) {
    return false;
  }

  auto vertex_input_bindings =
      shaders::model_renderer::Vertex::GetVertexInputBindings();
  auto vertex_input_attributes =
      shaders::model_renderer::Vertex::GetVertexInputAttributes();

  PipelineBuilder pipeline_builder;
  pipeline_builder.AddDynamicState(vk::DynamicState::eViewport);
  pipeline_builder.AddDynamicState(vk::DynamicState::eScissor);
  pipeline_builder.SetVertexInputDescription(vertex_input_bindings,
                                             vertex_input_attributes);
  pipeline_builder.SetFrontFace(vk::FrontFace::eCounterClockwise);

  for (const auto& topology : required_topologies) {
    pipeline_builder.SetPrimitiveTopology(topology);
    auto pipeline = pipeline_builder.CreatePipeline(
        GetContext().GetDevice(),                    //
        GetContext().GetPipelineCache(),             //
        pipeline_layout_.get(),                      //
        GetContext().GetOnScreenRenderPass(),        //
        library.GetPipelineShaderStageCreateInfos()  //
    );

    if (!pipeline) {
      return false;
    }

    pipelines_[topology] = std::move(pipeline);
  }

  uniform_buffer_ = {
      GetContext().GetMemoryAllocator(),     // allocator
      {},                                    // prototype
      GetContext().GetSwapchainImageCount()  // image count
  };

  auto buffer_infos = uniform_buffer_.GetBufferInfos();

  if (buffer_infos.size() != descriptor_sets_.GetSize()) {
    return false;
  }

  auto write_descriptor_set_generator = [&](size_t index) {
    return std::vector<vk::WriteDescriptorSet>{{
        nullptr,  // dst set (will be filled out later)
        0u,       // binding
        0u,       // array element
        1u,       // descriptor count
        vk::DescriptorType::eUniformBuffer,  // type
        nullptr,                             // image
        &buffer_infos[index],                // buffer
        nullptr,                             // buffer view
    }};
  };

  if (!descriptor_sets_.UpdateDescriptorSets(write_descriptor_set_generator)) {
    return false;
  }

  std::vector<shaders::model_renderer::Vertex> vertex_buffer;
  std::vector<uint32_t> index_buffer;

  vk::DeviceSize current_vertex_buffer_offset = 0;
  vk::DeviceSize current_index_buffer_offset = 0;
  std::vector<DrawData> pending_draw_data;

  for (const auto& op : draw_data.ops) {
    for (const auto& call : op.second) {
      DrawData data;
      data.pipeline = pipelines_[op.first].get();
      data.vertex_buffer_offset = current_vertex_buffer_offset;
      data.index_buffer_offset = current_index_buffer_offset;
      data.index_count = call.indices.size();
      std::copy(call.vertices.begin(), call.vertices.end(),
                std::back_inserter(vertex_buffer));
      std::copy(call.indices.begin(), call.indices.end(),
                std::back_inserter(index_buffer));
      current_vertex_buffer_offset +=
          call.vertices.size() * sizeof(decltype(call.vertices)::value_type);
      current_index_buffer_offset +=
          call.indices.size() * sizeof(decltype(call.indices)::value_type);
      pending_draw_data.push_back(data);
    }
  }

  vertex_buffer_ =
      GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
          vk::BufferUsageFlagBits::eVertexBuffer, vertex_buffer,
          GetContext().GetTransferCommandPool(), nullptr, nullptr, nullptr,
          nullptr);
  index_buffer_ = GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlagBits::eIndexBuffer, index_buffer,
      GetContext().GetTransferCommandPool(), nullptr, nullptr, nullptr,
      nullptr);

  GetContext().GetTransferQueue().queue.waitIdle();

  if (!vertex_buffer_ || !index_buffer_) {
    return false;
  }

  draw_data_ = std::move(pending_draw_data);

  return true;
}

// |Renderer|
bool ModelRenderer::Render(vk::CommandBuffer buffer) {
  if (draw_data_.empty()) {
    return true;
  }

  const auto extents = GetContext().GetExtents();

  auto model = glm::identity<glm::mat4>();
  auto view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f),  // eye
                          glm::vec3(0.0f),              // center
                          glm::vec3(0.0f, 0.0f, 1.0f)   // up
  );
  auto projection = glm::perspective(
      glm::radians(90.0f),
      static_cast<float>(extents.width) / static_cast<float>(extents.height),
      0.0f, 10.0f);

  uniform_buffer_->mvp = projection * view * model;

  if (!uniform_buffer_.UpdateUniformData()) {
    return false;
  }

  const auto uniform_index = uniform_buffer_.GetCurrentIndex();

  buffer.setScissor(0u, {GetContext().GetScissorRect()});
  buffer.setViewport(0u, {GetContext().GetViewport()});

  for (const auto& draw : draw_data_) {
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, draw.pipeline);
    buffer.bindVertexBuffers(0u, {vertex_buffer_->buffer},
                             {draw.vertex_buffer_offset});
    buffer.bindIndexBuffer(index_buffer_->buffer, draw.index_buffer_offset,
                           vk::IndexType::eUint32);
    buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,  // bind point
        pipeline_layout_.get(),            // layout
        0u,                                // first set
        descriptor_sets_[uniform_index],   // descriptor set
        nullptr                            // dynamic_offsets
    );
    buffer.drawIndexed(draw.index_count,  // index count
                       1u,                // instance count
                       0u,                // first index
                       0u,                // vertex offset
                       0u                 // first instance
    );
  }

  return true;
}

// |Renderer|
bool ModelRenderer::Teardown() {
  return true;
}

}  // namespace pixel
