#pragma once

#include "application.h"
#include "macros.h"
#include "renderer.h"
#include "rendering_context.h"

namespace pixel {

class PixelRenderer final : public Renderer {
 public:
  PixelRenderer(std::shared_ptr<RenderingContext> context);

  // |Renderer|
  ~PixelRenderer() override;

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

  Application::Object GetApplicationObject() const;

 private:
  std::shared_ptr<Application> application_;

  P_DISALLOW_COPY_AND_ASSIGN(PixelRenderer);
};

}  // namespace pixel
