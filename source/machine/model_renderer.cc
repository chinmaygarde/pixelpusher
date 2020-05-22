#include "model_renderer.h"

#include <future>

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

  descriptor_set_layout_ =
      shaders::model_renderer::UniformBuffer::CreateDescriptorSetLayout(
          GetContext().GetDevice());

  if (!descriptor_set_layout_) {
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

  if (!model_->PrepareToRender(GetContext())) {
    return false;
  }

  return true;
}

// |Renderer|
bool ModelRenderer::Render(vk::CommandBuffer render_command_buffer) {
  return false;
}

// |Renderer|
bool ModelRenderer::Teardown() {
  return true;
}

}  // namespace pixel
