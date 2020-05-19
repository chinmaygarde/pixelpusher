#include "model_renderer.h"

#include "asset_loader.h"

namespace pixel {

ModelRenderer::ModelRenderer(std::shared_ptr<RenderingContext> context,
                             std::string model_assets_dir,
                             std::string model_path)
    : Renderer(context) {
  AssetLoader::GetGlobal()->LoadAsset(model_assets_dir, model_path,
                                      [](std::unique_ptr<Asset> asset) {
                                        if (!asset) {
                                          return;
                                        }
                                      });
}

ModelRenderer::~ModelRenderer() = default;

// |Renderer|
bool ModelRenderer::IsValid() const {
  return true;
}

// |Renderer|
bool ModelRenderer::Setup() {
  return true;
}

// |Renderer|
bool ModelRenderer::Render(vk::CommandBuffer render_command_buffer) {
  return true;
}

// |Renderer|
bool ModelRenderer::Teardown() {
  return true;
}

}  // namespace pixel
