#pragma once

#include "command_buffer.h"
#include "command_pool.h"
#include "descriptor_pool.h"
#include "macros.h"
#include "memory_allocator.h"
#include "queue_selection.h"
#include "vulkan.h"

namespace pixel {

class RenderingContext {
 public:
  RenderingContext(vk::Instance instance,
                   vk::PhysicalDevice physical_device,
                   vk::Device logical_device,
                   QueueSelection graphics_queue,
                   QueueSelection transfer_queue,
                   vk::RenderPass onscreen_render_pass,
                   size_t swapchain_image_count,
                   const vk::PhysicalDeviceFeatures& features,
                   vk::Extent2D extents);

  ~RenderingContext();

  bool IsValid() const;

  vk::Instance GetInstance() const;

  vk::PhysicalDevice GetPhysicalDevice() const;

  vk::Device GetDevice() const;

  MemoryAllocator& GetMemoryAllocator() const;

  vk::PipelineCache GetPipelineCache() const;

  QueueSelection GetGraphicsQueue() const;

  QueueSelection GetTransferQueue() const;

  const CommandPool& GetGraphicsCommandPool() const;

  const CommandPool& GetTransferCommandPool() const;

  DescriptorPool& GetDescriptorPool() const;

  vk::RenderPass GetOnScreenRenderPass() const;

  // TODO: Get rid of this.
  size_t GetSwapchainImageCount() const;

  // TODO: Get rid of this.
  const vk::Extent2D& GetExtents() const;

  vk::Viewport GetViewport() const;

  vk::Rect2D GetScissorRect() const;

  const vk::PhysicalDeviceFeatures& GetFeatures() const;

 private:
  const vk::Instance instance_;
  const vk::PhysicalDevice physical_device_;
  const vk::Device device_;
  std::unique_ptr<MemoryAllocator> memory_allocator_;
  vk::UniquePipelineCache pipeline_cache_;
  const QueueSelection graphics_queue_;
  const QueueSelection transfer_queue_;
  std::shared_ptr<CommandPool> graphics_command_pool_;
  std::shared_ptr<CommandPool> transfer_command_pool_;
  std::unique_ptr<DescriptorPool> descriptor_pool_;
  vk::RenderPass onscreen_render_pass_;
  const size_t swapchain_image_count_;
  const vk::PhysicalDeviceFeatures features_;
  const vk::Extent2D extents_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(RenderingContext);
};

}  // namespace pixel
