#pragma once

#include "key_input.h"
#include "pointer_input.h"
#include "rendering_context.h"
#include "vulkan.h"

namespace pixel {

class Renderer : public KeyInputDelegate, public PointerInputDelegate {
 public:
  virtual ~Renderer();

  virtual bool IsValid() const = 0;

  virtual bool Setup() = 0;

  virtual bool BeginFrame() = 0;

  virtual bool RenderFrame(vk::CommandBuffer render_command_buffer) = 0;

  virtual bool Teardown() = 0;

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

 protected:
  Renderer(std::shared_ptr<RenderingContext> context);

  RenderingContext& GetContext() const;

 private:
  std::shared_ptr<RenderingContext> context_;
};

}  // namespace pixel
