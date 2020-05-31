#pragma once

#include <chrono>

#include "command_pool.h"
// TODO: This must be removed to make the main renderer GLFW agnostic.
#include "glfw.h"
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
  using Clock = std::chrono::high_resolution_clock;

  bool glfw_initialized_ = false;
  bool vulkan_initialized_ = false;
  bool is_valid_ = false;
  ImGuiContext* imgui_context_ = nullptr;
  Clock::time_point last_frame_begin_;
  std::vector<float> frame_times_millis_;
  size_t frames_rendered_ = 0;

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

  bool GatherAndRenderPerformanceMetrics();

  P_DISALLOW_COPY_AND_ASSIGN(ImguiRenderer);
};

}  // namespace pixel
