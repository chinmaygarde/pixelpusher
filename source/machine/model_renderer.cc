#include "model_renderer.h"

#include <algorithm>
#include <future>

#include <imgui.h>

#include "pipeline_builder.h"
#include "pipeline_layout.h"
#include "string_utils.h"

namespace pixel {

ModelRenderer::ModelRenderer(std::shared_ptr<RenderingContext> context,
                             std::string model_assets_dir,
                             std::string model_path,
                             std::string debug_name)
    : Renderer(context), debug_name_(std::move(debug_name)) {
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
      0.1f, 10.0f);

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
  }
}

bool ModelRenderer::RebuildPipelines() {
  if (required_topologies_.empty()) {
    return true;
  }
}

}  // namespace pixel
