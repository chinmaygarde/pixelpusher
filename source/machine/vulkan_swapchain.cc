#include "vulkan_swapchain.h"

#include <optional>

#include "logging.h"
#include "render_pass.h"
#include "vulkan.h"

namespace pixel {

vk::Extent2D VulkanSwapchain::ChooseSwapExtents(
    const vk::SurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  vk::Extent2D extent;
  extent.setWidth(std::clamp(800u, capabilities.minImageExtent.width,
                             capabilities.maxImageExtent.width));
  extent.setHeight(std::clamp(600u, capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width));
  return extent;
}

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

  for (size_t i = 0; i < swapchain_images.value.size(); i++) {
    const auto& image = swapchain_images.value[i];
    SetDebugNameF(device, image, "Swapchain Image %zu", i);
    view_create_info.setImage(image);
    auto image_view =
        UnwrapResult(device.createImageViewUnique(view_create_info));
    if (!image_view) {
      return std::nullopt;
    }
    SetDebugNameF(device, image_view.get(), "Swapchain Image View %zu", i);
    image_views.emplace_back(std::move(image_view));
  }

  return {std::move(image_views)};
}

static std::optional<std::vector<vk::UniqueFramebuffer>>
CreateSwapchainFramebuffers(const vk::Device& device,
                            const std::vector<vk::UniqueImageView>& image_views,
                            const vk::RenderPass& render_pass,
                            vk::Extent2D extents,
                            vk::ImageView depth_stencil_image) {
  std::vector<vk::UniqueFramebuffer> framebuffers;

  vk::FramebufferCreateInfo framebuffer_info;
  framebuffer_info.setRenderPass(render_pass);
  framebuffer_info.setAttachmentCount(1u);
  framebuffer_info.setWidth(extents.width);
  framebuffer_info.setHeight(extents.height);
  framebuffer_info.setLayers(1u);

  for (size_t i = 0; i < image_views.size(); i++) {
    vk::ImageView color_attachment_image = image_views[i].get();
    vk::ImageView attachment_images[] = {color_attachment_image,
                                         depth_stencil_image};
    framebuffer_info.setPAttachments(attachment_images);
    auto result = device.createFramebufferUnique(framebuffer_info);
    if (result.result != vk::Result::eSuccess) {
      P_ERROR << "Could not create framebuffer.";
      return std::nullopt;
    }
    SetDebugNameF(device, result.value.get(), "Swapchain Framebuffer %zu", i);
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

  auto command_buffers =
      UnwrapResult(device.allocateCommandBuffersUnique(buffer_info));
  if (command_buffers.size() != buffer_count) {
    P_ERROR << "Could not allocate command buffers.";
    return std::nullopt;
  }

  for (const auto& command_buffer : command_buffers) {
    SetDebugName(device, command_buffer.get(), "Swapchain Command Buffer");
  }

  return command_buffers;
}

static std::unique_ptr<ImageView> CreateDepthStencilImageView(
    std::shared_ptr<RenderingContext> context,
    const vk::Extent2D& extents) {
  auto depth_format = context->GetOptimalSupportedDepthAttachmentFormat();
  if (!depth_format.has_value()) {
    P_ERROR << "No supported depth/stencil attachment format found.";
    return {};
  }

  vk::ImageCreateInfo image_info = {
      {},                    // flags of vk::ImageCreateFlags
      vk::ImageType::e2D,    // image type
      depth_format.value(),  // format
      vk::Extent3D{extents.width, extents.height, 1},   // extents
      1,                                                // mip level
      1,                                                // array layers
      vk::SampleCountFlagBits::e1,                      // samples
      vk::ImageTiling::eOptimal,                        // tiling
      vk::ImageUsageFlagBits::eDepthStencilAttachment,  // usage
      vk::SharingMode::eExclusive,                      // sharing mode
      0u,                                               // queue families count
      {},  // queue families (sharing mode is not concurrent so ignored)
      vk::ImageLayout::eUndefined  // initial layout
  };

  auto image = context->GetMemoryAllocator().CreateImage(
      image_info, DefaultDeviceLocalAllocationCreateInfo(),
      "Swapchain Depth/Stencil Image");

  if (!image) {
    P_ERROR << "Could not create swapchain depth stencil image.";
    return {};
  }

  // TODO: Figure out what to do with the stencil.
  vk::ImageAspectFlags aspect_mask = vk::ImageAspectFlagBits::eDepth;

  vk::ImageViewCreateInfo image_view_create_info = {
      {},                      // flags
      image->image,            // image
      vk::ImageViewType::e2D,  // type
      depth_format.value(),    // format
      {},                      // component mapping
      vk::ImageSubresourceRange{
          aspect_mask,  // aspect mask
          0u,           // base mip level
          1u,           // level count
          0u,           // base array layer
          1u            // layer count
      }                 // subresource range
  };

  auto image_view = UnwrapResult(
      context->GetDevice().createImageViewUnique(image_view_create_info));
  if (!image_view) {
    P_ERROR << "Could not create image view.";
    return {};
  }

  SetDebugName(context->GetDevice(), image_view.get(),
               "Swapchain Depth/Stencil ImageView");

  auto image_view_wrapper =
      std::make_unique<ImageView>(std::move(image), std::move(image_view));
  if (!image_view_wrapper->IsValid()) {
    P_ERROR << "Could not create depth/stencil image view.";
    return {};
  }
  return image_view_wrapper;
  ;
}

VulkanSwapchain::VulkanSwapchain(
    Delegate& p_delegate,
    std::shared_ptr<RenderingContext> context,
    vk::SwapchainCreateInfoKHR p_swapchain_create_info,
    vk::Format p_swapchain_image_format)
    : delegate_(p_delegate),
      device_(context->GetDevice()),
      context_(context),
      swapchain_create_info_(p_swapchain_create_info),
      image_format_(p_swapchain_image_format),
      extents_(swapchain_create_info_.imageExtent) {
  if (!context_ | !context_->IsValid()) {
    P_ERROR << "Swapchain created with invalid rendering context.";
    return;
  }

  swapchain_ =
      UnwrapResult(device_.createSwapchainKHRUnique(swapchain_create_info_));
  if (!swapchain_) {
    P_ERROR << "Swapchain was invalid.";
    return;
  }

  SetDebugName(device_, swapchain_.get(), "Main Swapchain");

  auto image_views =
      CreateSwapchainImageViews(device_, swapchain_.get(), image_format_);
  if (!image_views.has_value()) {
    P_ERROR << "Could not create swapchain image views.";
    return;
  }

  render_pass_ = CreateRenderPass(device_, image_format_);
  if (!render_pass_) {
    P_ERROR << "Could not create swapchain render pass.";
    return;
  }

  depth_stencil_image_view_ = CreateDepthStencilImageView(context_, extents_);
  if (!depth_stencil_image_view_) {
    return;
  }

  auto frame_buffers =
      CreateSwapchainFramebuffers(device_,                                   //
                                  image_views.value(),                       //
                                  render_pass_.get(),                        //
                                  extents_,                                  //
                                  depth_stencil_image_view_->GetImageView()  //
      );

  if (!frame_buffers.has_value()) {
    P_ERROR << "Could not materialize all swapchain framebuffers.";
    return;
  }

  vk::CommandPoolCreateInfo pool_info;
  pool_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  pool_info.setQueueFamilyIndex(
      context_->GetGraphicsQueue().queue_family_index);
  command_pool_ = UnwrapResult(device_.createCommandPoolUnique(pool_info));
  if (!command_pool_) {
    P_ERROR << "Could not create command buffer pool.";
    return;
  }

  SetDebugName(device_, command_pool_.get(), "Swapchain Command Pool");

  auto command_buffers = CreateSwapchainCommandBuffers(
      device_, command_pool_.get(), frame_buffers.value().size());
  if (!command_buffers.has_value()) {
    P_ERROR << "Could not create swapchain command buffers.";
    return;
  }

  ready_to_render_semaphore_ =
      UnwrapResult(device_.createSemaphoreUnique(vk::SemaphoreCreateInfo{}));
  ready_to_present_semaphore_ =
      UnwrapResult(device_.createSemaphoreUnique(vk::SemaphoreCreateInfo{}));

  if (!ready_to_render_semaphore_ || !ready_to_present_semaphore_) {
    P_ERROR << "Could not create swapchain semaphores.";
    return;
  }

  SetDebugName(device_, ready_to_render_semaphore_.get(),
               "Swapchain Ready To Render Semaphore");
  SetDebugName(device_, ready_to_present_semaphore_.get(),
               "Swapchain Ready To Present Semaphore");

  image_views_ = std::move(image_views.value());
  frame_buffers_ = std::move(frame_buffers.value());
  command_buffers_ = std::move(command_buffers.value());

  is_valid_ = true;
}

static vk::SwapchainCreateInfoKHR VkSwapchainCreateInfoWithUpdatedExtents(
    const vk::SwapchainCreateInfoKHR& original_create_info,
    const vk::Extent2D& extents) {
  auto create_info = original_create_info;
  create_info.imageExtent = extents;
  return create_info;
}

VulkanSwapchain::VulkanSwapchain(const VulkanSwapchain& swapchain,
                                 vk::Extent2D new_extents)
    : VulkanSwapchain(swapchain.delegate_,  // delegate
                      swapchain.context_,   // context
                      VkSwapchainCreateInfoWithUpdatedExtents(
                          swapchain.swapchain_create_info_,
                          new_extents),        // swapchain_create_info
                      swapchain.image_format_  // swapchain_image_format
      ) {}

VulkanSwapchain::~VulkanSwapchain() = default;

bool VulkanSwapchain::IsValid() const {
  return is_valid_;
}

const vk::Extent2D& VulkanSwapchain::GetExtents() const {
  return extents_;
}

const vk::RenderPass& VulkanSwapchain::GetRenderPass() const {
  return render_pass_.get();
}

std::optional<vk::CommandBuffer> VulkanSwapchain::AcquireNextCommandBuffer() {
  if (!is_valid_) {
    P_ERROR << "Swapchain is no longer valid.";
    return std::nullopt;
  }

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

VulkanSwapchain::SubmitResult VulkanSwapchain::SubmitCommandBuffer(
    vk::CommandBuffer buffer) {
  if (!is_valid_) {
    P_ERROR << "Swapchain is no longer valid.";
    return SubmitResult::kFailure;
  }

  if (!pending_command_buffer_.has_value()) {
    P_ERROR << "There was no pending command buffer to submit.";
    return SubmitResult::kFailure;
  }

  if (pending_command_buffer_.value() != buffer) {
    P_ERROR << "Command buffer to submit was not the pending command buffer.";
    return SubmitResult::kFailure;
  }

  if (!FinalizeCommandBuffer(buffer)) {
    P_ERROR << "Could not finalize command buffer.";
    return SubmitResult::kFailure;
  }

  const auto graphics_queue = context_->GetGraphicsQueue().queue;

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
      graphics_queue.submit(submit_info, nullptr /* fences */);
  if (command_buffer_submit_result != vk::Result::eSuccess) {
    P_ERROR << "Could not submit the pending command buffer.";
    return SubmitResult::kFailure;
  }

  pending_command_buffer_.reset();

  // ---------------------------------------------------------------------------
  // Present the contents of the submitted command buffer.
  // ---------------------------------------------------------------------------

  if (!pending_swapchain_image_index_.has_value()) {
    P_ERROR
        << "Swapchain image index not associated with pending command buffer.";
    return SubmitResult::kFailure;
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

  auto present_result = graphics_queue.presentKHR(&present_info);
  switch (present_result) {
    case vk::Result::eSuccess:
      break;
    case vk::Result::eSuboptimalKHR:
    case vk::Result::eErrorOutOfDateKHR:
      delegate_.OnSwapchainNeedsRecreation(*this);
      return SubmitResult::kTryAgain;
    default:
      P_ERROR << "Could not present the swapchain image: "
              << to_string(present_result);
      return SubmitResult::kFailure;
  }

  pending_swapchain_image_index_.reset();

  // TODO: Remove temporary pessimization till multiple frames in flight are
  // implemented.
  if (graphics_queue.waitIdle() != vk::Result::eSuccess) {
    P_ERROR << "Could not wait the graphics queue to go idle.";
    return SubmitResult::kFailure;
  }

  return SubmitResult::kSuccess;
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

void VulkanSwapchain::Retire() {
  device_.waitIdle();

  is_valid_ = false;

  ready_to_present_semaphore_.reset();
  ready_to_render_semaphore_.reset();
  command_buffers_.clear();
  command_pool_.reset();
  render_pass_.reset();
  frame_buffers_.clear();
  image_views_.clear();
  swapchain_.reset();
}

}  // namespace pixel
