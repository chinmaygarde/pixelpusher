#include "vulkan_swapchain.h"

#include <optional>

#include "logging.h"
#include "render_pass.h"
#include "vulkan.h"

namespace pixel {

static std::optional<std::vector<vk::UniqueImageView>>
CreateSwapchainImageViews(const vk::Device& device,
                          const vk::SwapchainKHR& swapchain,
                          vk::Format swapchain_image_format) {
  auto swapchain_images = device.getSwapchainImagesKHR(swapchain);
  if (swapchain_images.result != vk::Result::eSuccess) {
    P_ERROR << "Could not get swapchain images.";
    return std::nullopt;
  }

  if (swapchain_images.value.size() == 0) {
    return std::nullopt;
  }

  std::vector<vk::UniqueImageView> image_views;

  vk::ImageViewCreateInfo view_create_info;

  view_create_info.setViewType(vk::ImageViewType::e2D);
  view_create_info.setFormat(swapchain_image_format);

  vk::ImageSubresourceRange range;
  range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  range.setBaseMipLevel(0u);
  range.setLevelCount(1u);
  range.setBaseArrayLayer(0u);
  range.setLayerCount(1u);

  view_create_info.setSubresourceRange(range);

  for (const auto& image : swapchain_images.value) {
    view_create_info.setImage(image);
    auto image_view = device.createImageViewUnique(view_create_info);
    if (image_view.result != vk::Result::eSuccess) {
      return std::nullopt;
    }
    image_views.emplace_back(std::move(image_view.value));
  }

  return {std::move(image_views)};
}

static std::optional<std::vector<vk::UniqueFramebuffer>>
CreateSwapchainFramebuffers(const vk::Device& device,
                            const std::vector<vk::UniqueImageView>& image_views,
                            const vk::RenderPass& render_pass,
                            vk::Extent2D extents) {
  std::vector<vk::UniqueFramebuffer> framebuffers;

  vk::FramebufferCreateInfo framebuffer_info;
  framebuffer_info.setRenderPass(render_pass);
  framebuffer_info.setAttachmentCount(1u);
  framebuffer_info.setWidth(extents.width);
  framebuffer_info.setHeight(extents.height);
  framebuffer_info.setLayers(1u);

  for (size_t i = 0; i < image_views.size(); i++) {
    vk::ImageView attachment = image_views[i].get();
    framebuffer_info.setPAttachments(&attachment);

    auto result = device.createFramebufferUnique(framebuffer_info);
    if (result.result != vk::Result::eSuccess) {
      P_ERROR << "Could not create framebuffer.";
      return std::nullopt;
    }

    framebuffers.emplace_back(std::move(result.value));
  }

  return {std::move(framebuffers)};
}

static std::optional<std::vector<vk::UniqueCommandBuffer>>
CreateSwapchainCommandBuffers(const vk::Device& device,
                              const vk::CommandPool& commandPool,
                              uint32_t buffer_count) {
  vk::CommandBufferAllocateInfo buffer_info;
  buffer_info.setCommandPool(commandPool);
  buffer_info.setLevel(vk::CommandBufferLevel::ePrimary);
  buffer_info.setCommandBufferCount(buffer_count);

  auto result = device.allocateCommandBuffersUnique(buffer_info);
  if (result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not allocate command buffers.";
    return std::nullopt;
  }

  if (result.value.size() != buffer_count) {
    P_ERROR << "Unexpected command buffers count.";
    return std::nullopt;
  }

  return std::move(result.value);
}

VulkanSwapchain::VulkanSwapchain(const vk::Device& device,
                                 vk::UniqueSwapchainKHR swapchain,
                                 vk::Format swapchain_image_format,
                                 uint32_t graphics_queue_family_index,
                                 vk::Extent2D extents)
    : extents_(extents) {
  if (!swapchain) {
    P_ERROR << "Swapchain was invalid.";
    return;
  }

  auto image_views = CreateSwapchainImageViews(device, swapchain.get(),
                                               swapchain_image_format);

  if (!image_views.has_value()) {
    P_ERROR << "Could not create swapchain image views.";
    return;
  }

  auto render_pass = CreateRenderPass(device, swapchain_image_format);

  if (!render_pass) {
    P_ERROR << "Could not create swapchain render pass.";
    return;
  }

  auto frame_buffers = CreateSwapchainFramebuffers(device,               //
                                                   image_views.value(),  //
                                                   render_pass.get(),    //
                                                   extents               //
  );

  if (!frame_buffers.has_value()) {
    P_ERROR << "Could not materialize all swapchain framebuffers.";
    return;
  }

  vk::CommandPoolCreateInfo pool_info;
  pool_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  pool_info.setQueueFamilyIndex(graphics_queue_family_index);
  auto pool_result = device.createCommandPoolUnique(pool_info);
  if (pool_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create command buffer pool.";
    return;
  }

  auto command_buffers = CreateSwapchainCommandBuffers(
      device, pool_result.value.get(), frame_buffers.value().size());
  if (!command_buffers.has_value()) {
    P_ERROR << "Could not create swapchain command buffers.";
    return;
  }

  auto render_semaphore_result =
      device.createSemaphoreUnique(vk::SemaphoreCreateInfo{});
  if (render_semaphore_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create semaphore";
    return;
  }

  auto present_semaphore_result =
      device.createSemaphoreUnique(vk::SemaphoreCreateInfo{});
  if (present_semaphore_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create semaphore";
    return;
  }

  auto graphics_queue =
      device.getQueue(graphics_queue_family_index,  // queue family index
                      0u                            // queue index
      );

  device_ = device;
  graphics_queue_ = graphics_queue;
  swapchain_ = std::move(swapchain);
  image_views_ = std::move(image_views.value());
  render_pass_ = std::move(render_pass);
  frame_buffers_ = std::move(frame_buffers.value());
  command_pool_ = std::move(pool_result.value);
  command_buffers_ = std::move(command_buffers.value());
  ready_to_render_semaphore_ = std::move(render_semaphore_result.value);
  ready_to_present_semaphore_ = std::move(present_semaphore_result.value);

  is_valid_ = true;
}

VulkanSwapchain::~VulkanSwapchain() = default;

bool VulkanSwapchain::IsValid() const {
  return is_valid_;
}

const vk::Extent2D& VulkanSwapchain::GetExtents() const {
  return extents_;
}

const vk::RenderPass& VulkanSwapchain::GetRenderPass() const {
  P_ASSERT(is_valid_);
  return render_pass_.get();
}

std::optional<vk::CommandBuffer> VulkanSwapchain::AcquireNextCommandBuffer() {
  P_ASSERT(is_valid_);
  if (pending_command_buffer_.has_value()) {
    P_ERROR << "A command buffer was already pending for submission.";
    return std::nullopt;
  }

  auto result = device_.acquireNextImageKHR(
      swapchain_.get(),                      // swapchain
      std::numeric_limits<uint64_t>::max(),  // timeout
      ready_to_render_semaphore_.get(),      // semaphore
      {}                                     // fence
  );
  if (result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not acquire next swapchain image.";
    return std::nullopt;
  }

  if (result.value >= command_buffers_.size()) {
    P_ERROR << "Swapchain image index returned was invalid.";
    return std::nullopt;
  }

  pending_swapchain_image_index_ = result.value;
  pending_command_buffer_ = command_buffers_[result.value].get();

  if (!PrepareCommandBuffer(pending_command_buffer_.value(), result.value)) {
    P_ERROR << "Could not prepare command buffer.";
    return std::nullopt;
  }

  return pending_command_buffer_.value();
}

bool VulkanSwapchain::SubmitCommandBuffer(vk::CommandBuffer buffer) {
  if (!pending_command_buffer_.has_value()) {
    P_ERROR << "There was no pending command buffer to submit.";
    return false;
  }

  if (pending_command_buffer_.value() != buffer) {
    P_ERROR << "Command buffer to submit was not the pending command buffer.";
    return false;
  }

  if (!FinalizeCommandBuffer(buffer)) {
    P_ERROR << "Could not finalize command buffer.";
    return false;
  }

  // ---------------------------------------------------------------------------
  // Submit the command buffer.
  // ---------------------------------------------------------------------------

  vk::SubmitInfo submit_info;

  submit_info.setPCommandBuffers(&buffer);
  submit_info.setCommandBufferCount(1u);

  // Wait until the slot is ready for rendering.
  submit_info.setPWaitSemaphores(&ready_to_render_semaphore_.get());
  submit_info.setWaitSemaphoreCount(1u);
  vk::PipelineStageFlags wait_stage =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;
  submit_info.setPWaitDstStageMask(&wait_stage);

  // Signal that the slot is ready for presentation.
  submit_info.setPSignalSemaphores(&ready_to_present_semaphore_.get());
  submit_info.setSignalSemaphoreCount(1u);

  auto command_buffer_submit_result =
      graphics_queue_.submit(submit_info, nullptr /* fences */);
  if (command_buffer_submit_result != vk::Result::eSuccess) {
    P_ERROR << "Could not submit the pending command buffer.";
    return false;
  }

  pending_command_buffer_.reset();

  // ---------------------------------------------------------------------------
  // Present the contents of the submitted command buffer.
  // ---------------------------------------------------------------------------

  if (!pending_swapchain_image_index_.has_value()) {
    P_ERROR
        << "Swapchain image index not associated with pending command buffer.";
    return false;
  }

  vk::PresentInfoKHR present_info;

  // Wait for the presentation to be ready.
  present_info.setPWaitSemaphores(&ready_to_present_semaphore_.get());
  present_info.setWaitSemaphoreCount(1u);

  // Select the swapchain to present on.
  present_info.setPSwapchains(&swapchain_.get());
  present_info.setSwapchainCount(1u);

  // Select the swapchain image to present to.
  present_info.setPImageIndices(&pending_swapchain_image_index_.value());

  auto present_result = graphics_queue_.presentKHR(&present_info);
  if (present_result != vk::Result::eSuccess) {
    P_ERROR << "Could not present the swapchain image.";
    return false;
  }

  pending_swapchain_image_index_.reset();

  // TODO: Remove temporary pessimization till multiple frames in flight are
  // implemented.
  if (graphics_queue_.waitIdle() != vk::Result::eSuccess) {
    P_ERROR << "Could not wait the graphics queue to go idle.";
    return false;
  }

  return true;
}

bool VulkanSwapchain::PrepareCommandBuffer(vk::CommandBuffer buffer,
                                           size_t swapchain_index) {
  if (!is_valid_) {
    P_ERROR << "Swapchain was not valid.";
    return false;
  }

  // Being recording the command buffer.
  auto result = buffer.begin(vk::CommandBufferBeginInfo{});
  if (result != vk::Result::eSuccess) {
    P_ERROR << "Could not begin recording the command buffer.";
    return false;
  }

  // Begin the render pass.
  vk::RenderPassBeginInfo render_pass_begin_info;
  render_pass_begin_info.setRenderPass(render_pass_.get());
  render_pass_begin_info.setFramebuffer(frame_buffers_[swapchain_index].get());
  render_pass_begin_info.setRenderArea({{0, 0}, extents_});
  render_pass_begin_info.setClearValueCount(1u);

  vk::ClearValue clear_color = {std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
  render_pass_begin_info.setPClearValues(&clear_color);

  buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

  return true;
}

bool VulkanSwapchain::FinalizeCommandBuffer(vk::CommandBuffer buffer) {
  // End the render pass.
  buffer.endRenderPass();

  // End the command buffer.
  auto buffer_end_result = buffer.end();
  if (buffer_end_result != vk::Result::eSuccess) {
    P_ERROR << "Could not end recording the command buffer.";
    return false;
  }

  return true;
}

size_t VulkanSwapchain::GetImageCount() const {
  return image_views_.size();
}

}  // namespace pixel
