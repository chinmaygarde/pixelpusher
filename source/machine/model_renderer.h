#pragma once

#include "asset_loader.h"
#include "macros.h"
#include "model.h"
#include "renderer.h"
#include "unshared_weak.h"

namespace pixel {

class ModelRenderer : public Renderer {
 public:
  ModelRenderer(std::shared_ptr<RenderingContext> context,
                std::string model_assets_dir,
                std::string model_path);

  // |Renderer|
  ~ModelRenderer() override;

 private:
  std::unique_ptr<model::Model> model_;
  bool is_valid_ = false;

  UnsharedWeakFactory<ModelRenderer> weak_factory_;

  // |Renderer|
  bool IsValid() const override;

  // |Renderer|
  bool Setup() override;

  // |Renderer|
  bool Render(vk::CommandBuffer render_command_buffer) override;

  // |Renderer|
  bool Teardown() override;

  P_DISALLOW_COPY_AND_ASSIGN(ModelRenderer);
};

}  // namespace pixel
