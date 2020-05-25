#include "main_renderer.h"

#include "assets_location.h"
#include "imgui_renderer.h"
#include "model_renderer.h"
#include "tutorial_renderer.h"
#include "vulkan_swapchain.h"

namespace pixel {

MainRenderer::MainRenderer(VulkanConnection& connection,
                           std::shared_ptr<RenderingContext> context,
                           GLFWwindow* window)
    : Renderer(context), connection_(connection) {
  is_valid_ = PushRenderer(std::make_unique<TutorialRenderer>(context)) &&
              PushRenderer(std::make_unique<ImguiRenderer>(context, window)) &&
              PushRenderer(std::make_unique<ModelRenderer>(
                  context, PIXEL_GLTF_MODELS_LOCATION "/Triangle/glTF",
                  "Triangle.gltf"));
}

MainRenderer::~MainRenderer() = default;

bool MainRenderer::PushRenderer(std::unique_ptr<Renderer> renderer) {
  if (!renderer || !renderer->IsValid()) {
    return false;
  }

  renderers_.emplace_back(std::move(renderer));
  return true;
}

std::unique_ptr<Renderer> MainRenderer::PopRenderer() {
  if (renderers_.empty()) {
    return nullptr;
  }

  auto top = std::move(renderers_.back());
  renderers_.pop_back();
  return top;
}

// |Renderer|
bool MainRenderer::IsValid() const {
  for (const auto& renderer : renderers_) {
    if (!renderer->IsValid()) {
      return false;
    }
  }
  return true;
}

// |Renderer|
bool MainRenderer::Setup() {
  for (const auto& renderer : renderers_) {
    if (!renderer->Setup()) {
      return false;
    }
  }
  return true;
}

// |Renderer|
bool MainRenderer::Render(vk::CommandBuffer render_command_buffer) {
  for (const auto& renderer : renderers_) {
    if (!renderer->Render(render_command_buffer)) {
      return false;
    }
  }
  return true;
}

// |Renderer|
bool MainRenderer::Teardown() {
  for (const auto& renderer : renderers_) {
    if (!renderer->Teardown()) {
      return false;
    }
  }
  return true;
}

bool MainRenderer::Render() {
  auto buffer = connection_.GetSwapchain().AcquireNextCommandBuffer();

  if (!buffer.has_value()) {
    return false;
  }

  GetContext().GetMemoryAllocator().TraceUsageStatistics();

  for (const auto& renderer : renderers_) {
    if (!renderer->Render(buffer.value())) {
      return false;
    }
  }

  if (!connection_.GetSwapchain().SubmitCommandBuffer(buffer.value())) {
    return false;
  }

  return true;
}

}  // namespace pixel
