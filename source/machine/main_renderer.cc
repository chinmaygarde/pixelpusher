#include "main_renderer.h"

#include "assets_location.h"
#include "imgui_renderer.h"
#include "model_renderer.h"
#include "tutorial_renderer.h"
#include "vulkan_swapchain.h"

namespace pixel {

#define MODEL_NAME "DamagedHelmet"

MainRenderer::MainRenderer(VulkanConnection& connection, GLFWwindow* window)
    : Renderer(connection.GetRenderingContext()), connection_(connection) {
  auto context = connection_.GetRenderingContext();
  is_valid_ = /* PushRenderer(std::make_unique<TutorialRenderer>(context)) && */
      PushRenderer(std::make_unique<ModelRenderer>(
          context, PIXEL_GLTF_MODELS_LOCATION "/" MODEL_NAME "/glTF",
          MODEL_NAME ".gltf", MODEL_NAME)) &&
      PushRenderer(std::make_unique<ImguiRenderer>(context, window));
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
bool MainRenderer::RenderFrame(vk::CommandBuffer render_command_buffer) {
  for (const auto& renderer : renderers_) {
    if (!renderer->RenderFrame(render_command_buffer)) {
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

// |Renderer|
bool MainRenderer::BeginFrame() {
  for (auto renderer = renderers_.rbegin(); renderer != renderers_.rend();
       ++renderer) {
    if (!(*renderer)->BeginFrame()) {
      return false;
    }
  }
  return true;
}

bool MainRenderer::Render() {
  while (true) {
    auto buffer = connection_.GetSwapchain().AcquireNextCommandBuffer();

    if (!buffer.has_value()) {
      return false;
    }

    if (!BeginFrame()) {
      P_ERROR << "Could not begin a new frame.";
      return false;
    }

    GetContext().GetMemoryAllocator().TraceUsageStatistics();

    if (!RenderFrame(buffer.value())) {
      P_ERROR << "Could not render frame.";
    }

    auto result =
        connection_.GetSwapchain().SubmitCommandBuffer(buffer.value());

    switch (result) {
      case VulkanSwapchain::SubmitResult::kSuccess:
        return true;
      case VulkanSwapchain::SubmitResult::kFailure:
        return false;
      case VulkanSwapchain::SubmitResult::kTryAgain:
        continue;
    }

    return false;
  }

  return true;
}

// |KeyInputDelegate|
bool MainRenderer::WantsKeyEvents() {
  return true;
}

// |KeyInputDelegate|
void MainRenderer::OnKeyEvent(KeyType type,
                              KeyAction action,
                              KeyModifiers modifiers) {
  for (const auto& renderer : renderers_) {
    if (renderer->WantsKeyEvents()) {
      renderer->OnKeyEvent(type, action, modifiers);
    }
  }
}

// |PointerInputDelegate|
bool MainRenderer::WantsPointerInput() {
  return true;
}

// |PointerInputDelegate|
bool MainRenderer::OnPointerEvent(int64_t pointer_id,
                                  PointerPhase phase,
                                  const PointerData& data) {
  for (auto i = renderers_.rbegin(); i != renderers_.rend(); i++) {
    if (!(*i)->WantsPointerInput()) {
      continue;
    }

    if ((*i)->OnPointerEvent(pointer_id, phase, data)) {
      return true;
    }
  }

  return false;
}

}  // namespace pixel
