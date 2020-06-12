#include "rendering_context.h"

#include <set>
#include <vector>

#include "logging.h"
#include "string_utils.h"

namespace pixel {

RenderingContext::RenderingContext(const Delegate& delegate,
                                   vk::Instance instance,
                                   vk::PhysicalDevice physical_device,
                                   vk::Device logical_device,
                                   QueueSelection graphics_queue,
                                   QueueSelection transfer_queue,
                                   const vk::PhysicalDeviceFeatures& features,
                                   const char* debug_name)
    : delegate_(delegate),
      instance_(instance),
      physical_device_(physical_device),
      device_(logical_device),
      graphics_queue_(graphics_queue),
      transfer_queue_(transfer_queue),
      features_(features) {
  if (!device_) {
    return;
  }

  memory_allocator_ =
      std::make_unique<MemoryAllocator>(physical_device, device_);
  if (!memory_allocator_->IsValid()) {
    return;
  }

  pipeline_cache_ = UnwrapResult(device_.createPipelineCacheUnique({}));
  if (!pipeline_cache_) {
    return;
  }

  SetDebugNameF(device_, pipeline_cache_.get(), "%s Pipeline Cache",
                debug_name);

  graphics_command_pool_ =
      CommandPool::Create(device_,                                        //
                          vk::CommandPoolCreateFlagBits::eTransient,      //
                          graphics_queue.queue_family_index,              //
                          graphics_queue.queue,                           //
                          MakeStringF("%s Graphics", debug_name).c_str()  //
      );
  if (!graphics_command_pool_) {
    return;
  }

  transfer_command_pool_ =
      CommandPool::Create(device_,                                        //
                          vk::CommandPoolCreateFlagBits::eTransient,      //
                          transfer_queue.queue_family_index,              //
                          transfer_queue.queue,                           //
                          MakeStringF("%s Transfer", debug_name).c_str()  //
      );
  if (!transfer_command_pool_) {
    return;
  }

  descriptor_pool_ = std::make_unique<DescriptorPool>(device_, debug_name);
  if (!descriptor_pool_) {
    return;
  }

  if (!ReadOptimalImageFormats()) {
    return;
  }

  is_valid_ = true;
}

RenderingContext::~RenderingContext() = default;

vk::Instance RenderingContext::GetInstance() const {
  return instance_;
}

vk::PhysicalDevice RenderingContext::GetPhysicalDevice() const {
  return physical_device_;
}

vk::Device RenderingContext::GetDevice() const {
  return device_;
}

bool RenderingContext::IsValid() const {
  return is_valid_;
}

QueueSelection RenderingContext::GetGraphicsQueue() const {
  return graphics_queue_;
}

QueueSelection RenderingContext::GetTransferQueue() const {
  return transfer_queue_;
}

MemoryAllocator& RenderingContext::GetMemoryAllocator() const {
  return *memory_allocator_;
}

vk::PipelineCache RenderingContext::GetPipelineCache() const {
  return *pipeline_cache_;
}

const CommandPool& RenderingContext::GetGraphicsCommandPool() const {
  return *graphics_command_pool_;
}

const CommandPool& RenderingContext::GetTransferCommandPool() const {
  return *transfer_command_pool_;
}

vk::RenderPass RenderingContext::GetOnScreenRenderPass() const {
  return delegate_.GetOnScreenRenderPass();
}

DescriptorPool& RenderingContext::GetDescriptorPool() const {
  return *descriptor_pool_;
}

size_t RenderingContext::GetSwapchainImageCount() const {
  return delegate_.GetSwapchainImageCount();
}

const vk::PhysicalDeviceFeatures& RenderingContext::GetFeatures() const {
  return features_;
}

vk::Extent2D RenderingContext::GetExtents() const {
  return delegate_.GetScreenExtents();
}

vk::Viewport RenderingContext::GetViewport() const {
  const auto extents = GetExtents();
  return {0,
          0,
          static_cast<float>(extents.width),
          static_cast<float>(extents.height),
          0.0,
          1.0};
}

vk::Rect2D RenderingContext::GetScissorRect() const {
  const auto extents = GetExtents();
  return {{0, 0}, {extents.width, extents.height}};
}

bool RenderingContext::FormatSupportsFeatures(
    vk::Format format,
    vk::FormatFeatureFlags buffer_features,
    vk::FormatFeatureFlags linear_tiling_features,
    vk::FormatFeatureFlags optimal_tiling_features) const {
  const auto properties = physical_device_.getFormatProperties(format);

  if (buffer_features && !(properties.bufferFeatures & buffer_features)) {
    return false;
  }

  if (linear_tiling_features &&
      !(properties.linearTilingFeatures & linear_tiling_features)) {
    return false;
  }

  if (optimal_tiling_features &&
      !(properties.optimalTilingFeatures & optimal_tiling_features)) {
    return false;
  }

  return true;
}

bool RenderingContext::ReadOptimalImageFormats() {
  auto add_if_supported = [&](ImageFormatKey key, vk::Format format) {
    if (optimal_image_formats_.count(key) != 0u) {
      // We already found a supported format for this key. No need to look
      // further.
      return;
    }
    if (FormatSupportsFeatures(
            format,                                   // format
            {},                                       // buffer
            {},                                       // linear
            vk::FormatFeatureFlagBits::eSampledImage  // optimal
            )) {
      optimal_image_formats_[key] = format;
    };
  };

  // Add more here are they are encountered. This list is a pretty conservative.
  add_if_supported({4, 8, ScalarFormat::kByte}, vk::Format::eR8G8B8A8Sint);
  add_if_supported({4, 8, ScalarFormat::kUnsignedByte},
                   vk::Format::eR8G8B8A8Uint);

  return !optimal_image_formats_.empty();
}

const std::vector<vk::Format> kKnownDepthStencilFormats = {
    vk::Format::eD24UnormS8Uint,   //
    vk::Format::eD16UnormS8Uint,   //
    vk::Format::eD32SfloatS8Uint,  //
    vk::Format::eD32Sfloat,        //
    vk::Format::eD16Unorm,         //
};

std::optional<vk::Format>
RenderingContext::GetOptimalSupportedDepthAttachmentFormat() const {
  for (const auto& depth_format : kKnownDepthStencilFormats) {
    if (FormatSupportsFeatures(
            depth_format,                                       // format
            {},                                                 // buffer
            {},                                                 // linear
            vk::FormatFeatureFlagBits::eDepthStencilAttachment  // optimal
            )) {
      return depth_format;
    }
  }

  return std::nullopt;
}

std::optional<vk::Format> RenderingContext::GetOptimalSampledImageFormat(
    size_t components,
    size_t bits_per_component,
    ScalarFormat component_format) const {
  ImageFormatKey key = {components, bits_per_component, component_format};

  auto found = optimal_image_formats_.find(key);

  if (found == optimal_image_formats_.end()) {
    P_ERROR << "No supported image format for an image with " << components
            << " component(s) " << bits_per_component
            << " bit(s) per component and a scalar format.";
    return std::nullopt;
  }

  return found->second;
}

}  // namespace pixel
