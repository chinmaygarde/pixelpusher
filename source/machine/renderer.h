#pragma once

#include "key_input.h"
#include "rendering_context.h"
#include "vulkan.h"

namespace pixel {

class Renderer : public KeyInputDelegate {
 public:
  virtual ~Renderer();

  virtual bool IsValid() const = 0;

  virtual bool Setup() = 0;

  virtual bool BeginFrame() = 0;

  virtual bool RenderFrame(vk::CommandBuffer render_command_buffer) = 0;

  virtual bool Teardown() = 0;

  // |KeyInputDelegate|
  virtual bool WantsKeyEvents();

  // |KeyInputDelegate|
  virtual void OnKeyEvent(KeyType type,
                          KeyAction action,
                          KeyModifiers modifiers);

 protected:
  Renderer(std::shared_ptr<RenderingContext> context);

  RenderingContext& GetContext() const;

 private:
  std::shared_ptr<RenderingContext> context_;
};

}  // namespace pixel
