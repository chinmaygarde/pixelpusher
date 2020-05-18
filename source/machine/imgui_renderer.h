#pragma once

#include "command_pool.h"
#include "imgui.h"
#include "macros.h"
#include "renderer.h"
#include "vulkan.h"

namespace pixel {

class ImguiRenderer final : public Renderer {
 public:
  ImguiRenderer(std::shared_ptr<RenderingContext> context, GLFWwindow* window);

  // |Renderer|
  ~ImguiRenderer() override;

 private:
  bool glfw_initialized_ = false;
  bool vulkan_initialized_ = false;
  bool is_valid_ = false;
  ImGuiContext* imgui_context_ = nullptr;

  // |Renderer|
  bool IsValid() const override;

  // |Renderer|
  bool Setup() override;

  // |Renderer|
  bool Render(vk::CommandBuffer render_command_buffer) override;

  // |Renderer|
  bool Teardown() override;

  bool RenderFrame(vk::CommandBuffer buffer) const;

  bool BeginFrame() const;

  P_DISALLOW_COPY_AND_ASSIGN(ImguiRenderer);
};

}  // namespace pixel
