#pragma once

#include "macros.h"
#include "vulkan.h"
//
#include "command_pool.h"
#include "imgui.h"

namespace pixel {

class ImguiConnection {
 public:
  ImguiConnection(GLFWwindow* window,
                  vk::Instance instance,
                  vk::PhysicalDevice physical_device,
                  vk::Device device,
                  uint32_t graphics_queue_family_index,
                  vk::PipelineCache pipeline_cache,
                  size_t swapchain_image_count,
                  vk::RenderPass onscreen_render_pass);

  ~ImguiConnection();

  bool IsValid() const;

  bool BeginFrame() const;

  bool RenderFrame(vk::CommandBuffer buffer) const;

 private:
  vk::UniqueDescriptorPool descriptor_pool_;
  std::shared_ptr<CommandPool> command_pool_;
  bool glfw_initialized_ = false;
  bool vulkan_initialized_ = false;
  bool is_valid_ = false;
  ImGuiContext* imgui_context_ = nullptr;

  P_DISALLOW_COPY_AND_ASSIGN(ImguiConnection);
};

}  // namespace pixel
