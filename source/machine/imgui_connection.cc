#include "imgui_connection.h"

#include "command_buffer.h"
#include "command_pool.h"
#include "imgui.h"
#include "macros.h"
#include "vulkan.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
//
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace pixel {

ImguiConnection::ImguiConnection(GLFWwindow* window,
                                 vk::Instance instance,
                                 vk::PhysicalDevice physical_device,
                                 vk::Device device,
                                 uint32_t graphics_queue_family_index,
                                 vk::PipelineCache pipeline_cache,
                                 size_t swapchain_image_count,
                                 vk::RenderPass onscreen_render_pass) {
  constexpr size_t kPoolSize = 1024;
  vk::DescriptorPoolCreateInfo pool_info;
  std::vector<vk::DescriptorPoolSize> pool_sizes = {
      {vk::DescriptorType::eSampler, kPoolSize},
      {vk::DescriptorType::eCombinedImageSampler, kPoolSize},
      {vk::DescriptorType::eSampledImage, kPoolSize},
      {vk::DescriptorType::eStorageImage, kPoolSize},
      {vk::DescriptorType::eUniformTexelBuffer, kPoolSize},
      {vk::DescriptorType::eStorageTexelBuffer, kPoolSize},
      {vk::DescriptorType::eUniformBuffer, kPoolSize},
      {vk::DescriptorType::eStorageBuffer, kPoolSize},
      {vk::DescriptorType::eUniformBufferDynamic, kPoolSize},
      {vk::DescriptorType::eStorageBufferDynamic, kPoolSize},
      {vk::DescriptorType::eInputAttachment, kPoolSize},
      {vk::DescriptorType::eInlineUniformBlockEXT, kPoolSize},
      {vk::DescriptorType::eAccelerationStructureKHR, kPoolSize},
      {vk::DescriptorType::eAccelerationStructureNV, kPoolSize},
  };
  pool_info.setMaxSets(pool_sizes.size() * kPoolSize);
  pool_info.setPoolSizeCount(pool_sizes.size());
  pool_info.setPPoolSizes(pool_sizes.data());

  descriptor_pool_ = UnwrapResult(device.createDescriptorPoolUnique(pool_info));

  if (!descriptor_pool_) {
    return;
  }

  SetDebugName(device, descriptor_pool_.get(), "ImGUI Descriptor Pool");

  imgui_context_ = ImGui::CreateContext();

  glfw_initialized_ = ::ImGui_ImplGlfw_InitForVulkan(window, true);
  if (!glfw_initialized_) {
    return;
  }

  ImGui_ImplVulkan_InitInfo vulkan_info = {};

  // TODO: Allow specification of a custom queue if necessary.
  auto queue = device.getQueue(graphics_queue_family_index, 0u);

  vulkan_info.Instance = instance;
  vulkan_info.PhysicalDevice = physical_device;
  vulkan_info.Device = device;
  vulkan_info.QueueFamily = graphics_queue_family_index;
  vulkan_info.Queue = queue;
  vulkan_info.PipelineCache = pipeline_cache;
  vulkan_info.DescriptorPool = descriptor_pool_.get();
  vulkan_info.MinImageCount = swapchain_image_count;
  vulkan_info.ImageCount = swapchain_image_count;
  vulkan_info.CheckVkResultFn = [](VkResult result) -> void {
    if (result != VK_SUCCESS) {
      P_ERROR << "Result of Vulkan call made by ImGui was not successful: "
              << result;
      P_ASSERT(false);
    }
  };
  static auto g_instance = instance;
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
  vulkan_initialized_ =
      ::ImGui_ImplVulkan_Init(&vulkan_info, onscreen_render_pass);

  if (!vulkan_initialized_) {
    return;
  }

  command_pool_ =
      CommandPool::Create(device, vk::CommandPoolCreateFlagBits::eTransient,
                          graphics_queue_family_index);

  SetDebugName(device, command_pool_->GetCommandPool(), "ImGui Command Pool");

  auto fonts_command_buffer = command_pool_->CreateCommandBuffer();

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

  device.waitIdle();

  ImGui_ImplVulkan_DestroyFontUploadObjects();

  is_valid_ = true;
}

ImguiConnection::~ImguiConnection() {
  if (vulkan_initialized_) {
    ::ImGui_ImplVulkan_Shutdown();
  }

  if (glfw_initialized_) {
    ::ImGui_ImplGlfw_Shutdown();
  }

  ImGui::DestroyContext(imgui_context_);
}

bool ImguiConnection::IsValid() const {
  return is_valid_;
}

bool ImguiConnection::BeginFrame() const {
  if (!IsValid()) {
    return false;
  }

  ::ImGui_ImplVulkan_NewFrame();
  ::ImGui_ImplGlfw_NewFrame();
  ::ImGui::NewFrame();

  return true;
}

bool ImguiConnection::RenderFrame(vk::CommandBuffer buffer) const {
  if (!IsValid()) {
    return false;
  }

  ::ImGui::Render();
  ::ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
  return true;
}

}  // namespace pixel
