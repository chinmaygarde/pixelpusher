#include "model_renderer.h"

#include <algorithm>
#include <future>

#include <imgui.h>

#include "pipeline_builder.h"
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
  required_topologies_ = draw_data.RequiredTopologies();

  shader_library_ = std::make_unique<ShaderLibrary>(GetContext().GetDevice());

  shader_library_->AddLiveUpdateCallback([this]() {
    // No need for weak because we own the shader library and it cannot outlast
    // us.
    this->OnShaderLibraryDidUpdate();
  });

  if (!shader_library_->AddDefaultVertexShader("model_renderer.vert") ||
      !shader_library_->AddDefaultFragmentShader("model_renderer.frag")) {
    return false;
  }

  descriptor_set_layout_ =
      shaders::model_renderer::UniformBuffer::CreateDescriptorSetLayout(
          GetContext().GetDevice());

  if (!descriptor_set_layout_) {
    return false;
  }

  descriptor_sets_ = GetContext().GetDescriptorPool().AllocateDescriptorSets(
      descriptor_set_layout_.get(), GetContext().GetSwapchainImageCount(),
      "Model Renderer");

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

  if (!RebuildPipelines()) {
    P_ERROR << "Could not rebuild pipelines.";
    return false;
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
      data.topology = op.first;
      data.vertex_buffer_offset = current_vertex_buffer_offset;
      data.index_buffer_offset = current_index_buffer_offset;
      data.index_count = call.indices.size();
      data.vertex_count = call.vertices.size();
      data.type = call.indices.empty() ? DrawType::kVertex : DrawType::kIndexed;
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

  if (!vertex_buffer.empty()) {
    vertex_buffer_ =
        GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
            vk::BufferUsageFlagBits::eVertexBuffer, vertex_buffer,
            GetContext().GetTransferCommandPool(), nullptr, nullptr, nullptr,
            nullptr);
    if (!vertex_buffer_) {
      return false;
    }

    SetDebugName(GetContext().GetDevice(), vertex_buffer_->buffer,
                 "Model Renderer Vertices");
  }

  if (!index_buffer.empty()) {
    index_buffer_ =
        GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
            vk::BufferUsageFlagBits::eIndexBuffer, index_buffer,
            GetContext().GetTransferCommandPool(), nullptr, nullptr, nullptr,
            nullptr);

    if (!index_buffer_) {
      return false;
    }

    SetDebugName(GetContext().GetDevice(), index_buffer_->buffer,
                 "Model Renderer Indices");
  }

  GetContext().GetTransferQueue().queue.waitIdle();

  draw_data_ = std::move(pending_draw_data);

  return true;
}

// |Renderer|
bool ModelRenderer::BeginFrame() {
  // Nothing to do here.
  return true;
}

// |Renderer|
bool ModelRenderer::RenderFrame(vk::CommandBuffer buffer) {
  if (::ImGui::BeginTabItem("Model")) {
    ::ImGui::Text("Draws: %zu", draw_data_.size());

    if (vertex_buffer_) {
      ::ImGui::Text("Vertex Buffer Size: %zu", vertex_buffer_->GetSize());
    } else {
      ::ImGui::Text("No Vertex Buffer");
    }

    if (index_buffer_) {
      ::ImGui::Text("Index Buffer Size: %zu", index_buffer_->GetSize());
    } else {
      ::ImGui::Text("No Index Buffer");
    }

    ::ImGui::Text("Topologies: %zu", required_topologies_.size());

    ::ImGui::EndTabItem();
  }

  if (draw_data_.empty()) {
    return true;
  }

  if (!vertex_buffer_) {
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
    auto found = pipelines_.find(draw.topology);
    if (found == pipelines_.end()) {
      return false;
    }

    const auto is_indexed_draw = draw.type == DrawType::kIndexed;

    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, found->second.get());
    buffer.bindVertexBuffers(0u, {vertex_buffer_->buffer},
                             {draw.vertex_buffer_offset});
    buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,  // bind point
        pipeline_layout_.get(),            // layout
        0u,                                // first set
        descriptor_sets_[uniform_index],   // descriptor set
        nullptr                            // dynamic_offsets
    );

    if (is_indexed_draw) {
      if (!index_buffer_) {
        return false;
      }

      buffer.bindIndexBuffer(index_buffer_->buffer, draw.index_buffer_offset,
                             vk::IndexType::eUint32);
      buffer.drawIndexed(draw.index_count,  // index count
                         1u,                // instance count
                         0u,                // first index
                         0u,                // vertex offset
                         0u                 // first instance
      );
    } else {
      buffer.draw(draw.vertex_count,  // vertex count
                  1u,                 // instance count
                  0u,                 // first vertex
                  0u                  // first instance
      );
    }
  }

  return true;
}

// |Renderer|
bool ModelRenderer::Teardown() {
  return true;
}

void ModelRenderer::OnShaderLibraryDidUpdate() {
  P_LOG << "Model renderer shader library did update.";

  if (!is_valid_) {
    return;
  }

  // We are going to be tearing down existing pipelines. Frames in flight must
  // be flushed.
  GetContext().GetDevice().waitIdle();

  if (!RebuildPipelines()) {
    P_ERROR << "Error while rebuilding pipelines. Renderer is no longer "
               "valid.";
    is_valid_ = false;
  }
}

bool ModelRenderer::RebuildPipelines() {
  if (required_topologies_.empty()) {
    return true;
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

  for (const auto& topology : required_topologies_) {
    pipeline_builder.SetPrimitiveTopology(topology);
    auto pipeline = pipeline_builder.CreatePipeline(
        GetContext().GetDevice(),                             //
        GetContext().GetPipelineCache(),                      //
        pipeline_layout_.get(),                               //
        GetContext().GetOnScreenRenderPass(),                 //
        shader_library_->GetPipelineShaderStageCreateInfos()  //
    );

    if (!pipeline) {
      return false;
    }

    SetDebugName(GetContext().GetDevice(), pipeline.get(),
                 "Model Renderer Pipeline");

    pipelines_[topology] = std::move(pipeline);
  }
  return true;
}

}  // namespace pixel
