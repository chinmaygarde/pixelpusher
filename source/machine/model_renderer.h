#pragma once

#include <chrono>

#include "macros.h"
#include "model.h"
#include "model_draw_data.h"
#include "renderer.h"
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
  std::chrono::high_resolution_clock::time_point start_time_ =
      std::chrono::high_resolution_clock::now();

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
