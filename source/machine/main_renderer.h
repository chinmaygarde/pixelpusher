#pragma once

#include <memory>
#include <vector>

// TODO: This must be removed to make the main renderer GLFW agnostic.
#include "glfw.h"
#include "macros.h"
#include "renderer.h"
#include "rendering_context.h"
#include "vulkan_connection.h"

namespace pixel {

class MainRenderer final : public Renderer {
 public:
  MainRenderer(VulkanConnection& connection,
               std::shared_ptr<RenderingContext> context,
               GLFWwindow* window);

  ~MainRenderer();

  bool Render();

  // |Renderer|
  bool IsValid() const override;

  // |Renderer|
  bool Setup() override;

  // |Renderer|
  bool Teardown() override;

 private:
  VulkanConnection& connection_;
  std::vector<std::unique_ptr<Renderer>> renderers_;
  bool is_valid_ = false;

  bool PushRenderer(std::unique_ptr<Renderer> renderer);

  std::unique_ptr<Renderer> PopRenderer();

  // |Renderer|
  bool Render(vk::CommandBuffer render_command_buffer) override;

  P_DISALLOW_COPY_AND_ASSIGN(MainRenderer);
};

}  // namespace pixel
