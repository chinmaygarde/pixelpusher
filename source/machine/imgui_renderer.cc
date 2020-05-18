#include "imgui_renderer.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "command_buffer.h"
#include "command_pool.h"
#include "imgui.h"
#include "macros.h"
#include "vulkan.h"

namespace pixel {

ImguiRenderer::ImguiRenderer(std::shared_ptr<RenderingContext> context,
                             GLFWwindow* window)
    : Renderer(context) {
  if (!context) {
    return;
  }

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
  if (!IsValid()) {
    return false;
  }

  // Nothing to do here except to begin a new frame. Setup already done in
  // constructor.
  return BeginFrame();
}

// |Renderer|
bool ImguiRenderer::Render(vk::CommandBuffer render_command_buffer) {
  return RenderFrame(render_command_buffer) && BeginFrame();
}

// |Renderer|
bool ImguiRenderer::Teardown() {
  // Nothing to do here. Destructor performs all cleanup.
  return true;
}

bool ImguiRenderer::BeginFrame() const {
  if (!IsValid()) {
    return false;
  }

  ::ImGui_ImplVulkan_NewFrame();
  ::ImGui_ImplGlfw_NewFrame();
  ::ImGui::NewFrame();

  return true;
}

bool ImguiRenderer::RenderFrame(vk::CommandBuffer buffer) const {
  if (!IsValid()) {
    return false;
  }

  ::ImGui::Render();
  ::ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
  return true;
}

}  // namespace pixel
