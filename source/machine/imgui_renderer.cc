#include "imgui_renderer.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <algorithm>

#include "command_buffer.h"
#include "command_pool.h"
#include "imgui.h"
#include "macros.h"
#include "vulkan.h"

namespace pixel {

constexpr size_t kFrameSamplesCount = 1500u;

ImguiRenderer::ImguiRenderer(std::shared_ptr<RenderingContext> context,
                             GLFWwindow* window)
    : Renderer(context) {
  if (!context) {
    return;
  }

  frame_times_millis_.resize(kFrameSamplesCount, 0.0f);

  imgui_context_ = ImGui::CreateContext();

  glfw_initialized_ = ::ImGui_ImplGlfw_InitForVulkan(window, true);
  if (!glfw_initialized_) {
    return;
  }

  ImGui_ImplVulkan_InitInfo vulkan_info = {};

  vulkan_info.Instance = GetContext().GetInstance();
  vulkan_info.PhysicalDevice = GetContext().GetPhysicalDevice();
  vulkan_info.Device = GetContext().GetDevice();
  vulkan_info.QueueFamily = GetContext().GetGraphicsQueue().queue_family_index;
  vulkan_info.Queue = GetContext().GetGraphicsQueue().queue;
  vulkan_info.PipelineCache = GetContext().GetPipelineCache();
  vulkan_info.DescriptorPool = GetContext().GetDescriptorPool().GetPool();
  vulkan_info.MinImageCount = GetContext().GetSwapchainImageCount();
  vulkan_info.ImageCount = GetContext().GetSwapchainImageCount();
  vulkan_info.CheckVkResultFn = [](VkResult result) -> void {
    if (result != VK_SUCCESS) {
      P_ERROR << "Result of Vulkan call made by ImGui was not successful: "
              << result;
      P_ASSERT(false);
    }
  };
  static auto g_instance = GetContext().GetInstance();
  vulkan_info.GetVulkanProcAddressFn =
      [](const char* function_name) -> PFN_vkVoidFunction {
    auto address = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
        g_instance, function_name);
    if (address == nullptr) {
      P_ERROR << "Could not get proc address for " << function_name;
      return nullptr;
    }
    return address;
  };
  vulkan_initialized_ = ::ImGui_ImplVulkan_Init(
      &vulkan_info, GetContext().GetOnScreenRenderPass());

  if (!vulkan_initialized_) {
    return;
  }

  auto fonts_command_buffer =
      GetContext().GetTransferCommandPool().CreateCommandBuffer();

  if (!fonts_command_buffer) {
    return;
  }

  {
    vk::CommandBufferBeginInfo begin_info;
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    fonts_command_buffer->GetCommandBuffer().begin(begin_info);
  }

  if (!ImGui_ImplVulkan_CreateFontsTexture(
          fonts_command_buffer->GetCommandBuffer())) {
    return;
  }

  { fonts_command_buffer->GetCommandBuffer().end(); }

  if (!fonts_command_buffer->Submit()) {
    return;
  }

  GetContext().GetDevice().waitIdle();

  ImGui_ImplVulkan_DestroyFontUploadObjects();

  is_valid_ = true;
}

ImguiRenderer::~ImguiRenderer() {
  if (vulkan_initialized_) {
    ::ImGui_ImplVulkan_Shutdown();
  }

  if (glfw_initialized_) {
    ::ImGui_ImplGlfw_Shutdown();
  }

  ImGui::DestroyContext(imgui_context_);
}

bool ImguiRenderer::IsValid() const {
  return is_valid_;
}

// |Renderer|
bool ImguiRenderer::Setup() {
  // Nothing to do here.
  return IsValid();
}

// |Renderer|
bool ImguiRenderer::BeginFrame() {
  if (!IsValid()) {
    return false;
  }

  ::ImGui_ImplVulkan_NewFrame();
  ::ImGui_ImplGlfw_NewFrame();
  ::ImGui::NewFrame();

  return true;
}

bool ImguiRenderer::RenderPerformanceMetrics() const {
  ::ImGui::Begin("Timing");
#ifndef NDEBUG
  ::ImGui::Text("WARNING: Debug/Unoptimized Renderer");
#endif  //  NDEBUG
  const auto last = frame_times_millis_[frames_rendered_ % kFrameSamplesCount];
  const auto max =
      *std::max_element(frame_times_millis_.begin(), frame_times_millis_.end());
  const auto min =
      *std::min_element(frame_times_millis_.begin(), frame_times_millis_.end());
  ::ImGui::LabelText("(Last, Max, Min) Frame Time", "(%.2f, %.2f, %.2f) ms",
                     last, max, min);
  ::ImGui::LabelText("(Last, Max, Min) FPS", "(%.0f, %.0f, %.0f) ms",
                     1000.f / last, 1000.f / min, 1000.f / max);
  ::ImGui::PlotLines("Frame History (ms)", frame_times_millis_.data(),
                     frame_times_millis_.size());
  ::ImGui::End();

  ::ImGui::ShowDemoWindow();
  return true;
}

// |Renderer|
bool ImguiRenderer::Teardown() {
  // Nothing to do here. Destructor performs all cleanup.
  return true;
}

// |Renderer|
bool ImguiRenderer::RenderFrame(vk::CommandBuffer buffer) {
  if (!IsValid()) {
    return false;
  }

  frames_rendered_++;
  auto now = Clock::now();
  std::chrono::duration<float, std::milli> duration = now - last_frame_begin_;
  frame_times_millis_[frames_rendered_ % kFrameSamplesCount] = duration.count();
  last_frame_begin_ = now;

  if (!RenderPerformanceMetrics()) {
    return false;
  }

  ::ImGui::Render();
  ::ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
  return true;
}

}  // namespace pixel
