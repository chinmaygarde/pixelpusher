#pragma once

#include <map>

#include "asset_loader.h"
#include "macros.h"
#include "model.h"
#include "model_draw_data.h"
#include "renderer.h"
#include "shader_library.h"
#include "unshared_weak.h"
#include "vulkan.h"

namespace pixel {

class ModelRenderer : public Renderer {
 public:
  ModelRenderer(std::shared_ptr<RenderingContext> context,
                std::string model_assets_dir,
                std::string model_path,
                std::string debug_name);

  // |Renderer|
  ~ModelRenderer() override;

 private:
  const std::string debug_name_;
  std::unique_ptr<model::ModelDeviceContext> model_device_context_;
  bool is_valid_ = false;

  // |Renderer|
  bool IsValid() const override;

  // |Renderer|
  bool Setup() override;

  // |Renderer|
  bool BeginFrame() override;

  // |Renderer|
  bool RenderFrame(vk::CommandBuffer render_command_buffer) override;

  // |Renderer|
  bool Teardown() override;

  P_DISALLOW_COPY_AND_ASSIGN(ModelRenderer);
};

}  // namespace pixel
