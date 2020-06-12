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
  class Delegate {
   public:
    virtual vk::RenderPass GetOnScreenRenderPass() const = 0;

    virtual size_t GetSwapchainImageCount() const = 0;

    virtual vk::Extent2D GetScreenExtents() const = 0;
  };

  RenderingContext(const Delegate& delegate,
                   vk::Instance instance,
                   vk::PhysicalDevice physical_device,
                   vk::Device logical_device,
                   QueueSelection graphics_queue,
                   QueueSelection transfer_queue,
                   const vk::PhysicalDeviceFeatures& features,
                   const char* debug_name);

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

  size_t GetSwapchainImageCount() const;

  vk::Extent2D GetExtents() const;

  vk::Viewport GetViewport() const;

  vk::Rect2D GetScissorRect() const;

  const vk::PhysicalDeviceFeatures& GetFeatures() const;

  bool FormatSupportsFeatures(
      vk::Format format,
      vk::FormatFeatureFlags buffer_features,
      vk::FormatFeatureFlags linear_tiling_features,
      vk::FormatFeatureFlags optimal_tiling_features) const;

  std::optional<vk::Format> GetOptimalSupportedDepthAttachmentFormat() const;

  std::optional<vk::Format> GetImageFormatForHostImageAllocation(
      size_t components,
      size_t bits_per_component,
      ScalarFormat component_format) const;

 private:
  const Delegate& delegate_;
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
  const vk::PhysicalDeviceFeatures features_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(RenderingContext);
};

}  // namespace pixel
