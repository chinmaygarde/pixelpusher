#include "pixel_renderer.h"

namespace pixel {

PixelRenderer::PixelRenderer(std::shared_ptr<RenderingContext> context)
    : Renderer(std::move(context)),
      application_(std::make_shared<Application>()) {}

// |Renderer|
PixelRenderer::~PixelRenderer() = default;

// |Renderer|
bool PixelRenderer::IsValid() const {
  return true;
}

// |Renderer|
bool PixelRenderer::Setup() {
  return true;
}

// |Renderer|
bool PixelRenderer::BeginFrame() {
  return true;
}

// |Renderer|
bool PixelRenderer::RenderFrame(vk::CommandBuffer render_command_buffer) {
  return true;
}

// |Renderer|
bool PixelRenderer::Teardown() {
  return true;
}

std::shared_ptr<Application> PixelRenderer::GetApplication() const {
  return application_;
}

}  // namespace pixel
