#pragma once

#include "rendering_context.h"
#include "vulkan.h"

namespace pixel {

class Renderer {
 public:
  virtual ~Renderer();

  virtual bool IsValid() const = 0;

  virtual bool Setup() = 0;

  virtual bool BeginFrame() = 0;

  virtual bool RenderFrame(vk::CommandBuffer render_command_buffer) = 0;

  virtual bool Teardown() = 0;

 protected:
  Renderer(std::shared_ptr<RenderingContext> context);

  RenderingContext& GetContext() const;

 private:
  std::shared_ptr<RenderingContext> context_;
};

}  // namespace pixel
