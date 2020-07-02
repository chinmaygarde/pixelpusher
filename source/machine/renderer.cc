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

// KeyInputDelegate
bool Renderer::WantsKeyEvents() {
  return false;
}

// KeyInputDelegate
void Renderer::OnKeyEvent(KeyType type,
                          KeyAction action,
                          KeyModifiers modifiers) {
  //
}

// |PointerInputDelegate|
bool Renderer::WantsPointerInput() {
  return false;
}

// |PointerInputDelegate|
bool Renderer::OnPointerEvent(int64_t pointer_id,
                              PointerPhase phase,
                              const PointerData& data) {
  return false;
}

}  // namespace pixel
