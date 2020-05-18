#include "renderer.h"

#include "logging.h"

namespace pixel {

Renderer::Renderer(std::shared_ptr<RenderingContext> context)
    : context_(std::move(context)) {
  P_ASSERT(context_);
}

Renderer::~Renderer() = default;

RenderingContext& Renderer::GetContext() const {
  return *context_;
}

}  // namespace pixel
