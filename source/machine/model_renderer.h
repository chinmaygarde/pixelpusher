#pragma once

#include <chrono>

#include "macros.h"
#include "matrix_simulation.h"
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
  MatrixSimulation view_xformation_;
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

  // |KeyInputDelegate|
  bool WantsKeyEvents() override;

  // |KeyInputDelegate|
  void OnKeyEvent(KeyType type,
                  KeyAction action,
                  KeyModifiers modifiers) override;

  // |PointerInputDelegate|
  bool WantsPointerInput() override;

  // |PointerInputDelegate|
  bool OnPointerEvent(int64_t pointer_id, Point point, Offset offset) override;

  P_DISALLOW_COPY_AND_ASSIGN(ModelRenderer);
};

}  // namespace pixel
