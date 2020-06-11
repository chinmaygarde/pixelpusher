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
  std::promise<std::unique_ptr<model::ModelDrawData>> model_draw_data_promise;
  auto model_draw_data_future = model_draw_data_promise.get_future();
  AssetLoader::GetGlobal()->LoadAsset(
      model_assets_dir, model_path,
      MakeCopyable(
          [promise = std::move(model_draw_data_promise),
           debug_name = debug_name_](std::unique_ptr<Asset> asset) mutable {
            if (!asset) {
              promise.set_value(nullptr);
              return;
            }
            auto model = std::make_unique<model::Model>(*asset);
            auto draw_data = model->CreateDrawData(debug_name);
            promise.set_value(std::move(draw_data));
          }));
  auto draw_data = model_draw_data_future.get();
  if (!draw_data) {
    P_ERROR << "Draw data was invalid.";
    return;
  }

  auto model_device_context = draw_data->CreateModelDeviceContext(context);
  if (!model_device_context) {
    P_ERROR << "Could not create model device context.";
    return;
  }

  model_device_context_ = std::move(model_device_context);

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

  return true;
}

// |Renderer|
bool ModelRenderer::BeginFrame() {
  // Nothing to do here.
  return true;
}

// |Renderer|
bool ModelRenderer::RenderFrame(vk::CommandBuffer buffer) {
  if (!is_valid_) {
    return false;
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

  model_device_context_->GetUniformBuffer().prototype.mvp =
      projection * view * model;

  return true;
}

// |Renderer|
bool ModelRenderer::Teardown() {
  return true;
}

}  // namespace pixel
