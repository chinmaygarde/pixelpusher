#include "model_renderer.h"

#include <future>

namespace pixel {

ModelRenderer::ModelRenderer(std::shared_ptr<RenderingContext> context,
                             std::string model_assets_dir,
                             std::string model_path)
    : Renderer(context), weak_factory_(this) {
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

  if (!model_->PrepareToRender(GetContext())) {
    return false;
  }

  return true;
}

// |Renderer|
bool ModelRenderer::Render(vk::CommandBuffer render_command_buffer) {
  if (!is_valid_) {
    return false;
  }

  if (!model_->Render(GetContext(), render_command_buffer)) {
    return true;
  }

  // |Renderer|
  bool ModelRenderer::Teardown() { return true; }

}  // namespace pixel
