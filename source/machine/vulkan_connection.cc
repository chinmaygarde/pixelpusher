#include "vulkan_connection.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <set>
#include <string>

#include "event_loop.h"
#include "logging.h"
#include "macros.h"
#include "vulkan_swapchain.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace pixel {

static const std::vector<const char*> kRequiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct SwapchainDetails {
  VkSurfaceCapabilitiesKHR capabilities = {};
  std::vector<vk::SurfaceFormatKHR> surface_formats;
  std::vector<vk::PresentModeKHR> present_modes;

  std::optional<vk::SurfaceFormatKHR> ChooseSurfaceFormat() const {
    for (const auto& surface_format : surface_formats) {
      if (surface_format.format == vk::Format::eB8G8R8A8Unorm &&
          surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        return surface_format;
      }
    }
    return std::nullopt;
  }

  std::optional<vk::PresentModeKHR> ChoosePresentMode() const {
    for (const auto& present_mode : present_modes) {
      if (present_mode == vk::PresentModeKHR::eMailbox) {
        return present_mode;
      }
    }
    return vk::PresentModeKHR::eFifo;
  }

  uint32_t ChooseSwapchainImageCount() const {
    return std::clamp<uint32_t>(3, capabilities.minImageCount,
                                capabilities.maxImageCount);
  };

  std::unique_ptr<VulkanSwapchain> CreateSwapchain(
      VulkanSwapchain::Delegate& delegate,
      vk::Device device,
      vk::SurfaceKHR surface,
      uint32_t graphics_family_index,
      uint32_t present_family_index) const {
    auto surface_format = ChooseSurfaceFormat();
    auto present_mode = ChoosePresentMode();
    auto swap_extent = VulkanSwapchain::ChooseSwapExtents(capabilities);

    if (!surface_format || !present_mode) {
      return nullptr;
    }

    vk::SwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info.setSurface(surface);
    swapchain_create_info.setMinImageCount(ChooseSwapchainImageCount());
    swapchain_create_info.setImageFormat(surface_format.value().format);
    swapchain_create_info.setImageColorSpace(surface_format.value().colorSpace);
    swapchain_create_info.setPresentMode(present_mode.value());
    swapchain_create_info.setImageExtent(swap_extent);
    swapchain_create_info.setImageArrayLayers(1u);
    swapchain_create_info.setImageUsage(
        vk::ImageUsageFlagBits::eColorAttachment);

    if (graphics_family_index != present_family_index) {
      swapchain_create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
      std::vector<uint32_t> shared_queues = {graphics_family_index,
                                             present_family_index};
      swapchain_create_info.setQueueFamilyIndexCount(shared_queues.size());
      swapchain_create_info.setPQueueFamilyIndices(shared_queues.data());
    } else {
      // Almost always the case that the graphics family is the same as the
      // present family.
      swapchain_create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    // TODO: Support transformation if necessary.
    swapchain_create_info.compositeAlpha =
        vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchain_create_info.setClipped(true);

    auto vulkan_swapchain = std::make_unique<VulkanSwapchain>(
        delegate,
        device,                                     //
        swapchain_create_info,                      //
        surface_format.value().format,              //
        graphics_family_index,                      //
        device.getQueue(graphics_family_index, 0u)  //
    );

    if (!vulkan_swapchain->IsValid()) {
      return nullptr;
    }

    return vulkan_swapchain;
  }
};

struct PhysicalDeviceSelection {
  std::optional<uint32_t> device_index;
  std::optional<uint32_t> graphics_family_index;
  std::optional<uint32_t> present_family_index;
  bool has_required_extensions = false;
  std::optional<SwapchainDetails> swapchain_details;

  bool IsValid() const {
    return graphics_family_index.has_value() &&
           present_family_index.has_value() && device_index.has_value() &&
           has_required_extensions && swapchain_details.has_value();
  }

  operator bool() const { return IsValid(); }

  std::unique_ptr<VulkanSwapchain> CreateSwapchain(
      VulkanSwapchain::Delegate& delegate,
      vk::Device device,
      vk::SurfaceKHR surface) {
    if (!IsValid()) {
      return nullptr;
    }

    return swapchain_details.value().CreateSwapchain(
        delegate,                       //
        device,                         //
        surface,                        //
        graphics_family_index.value(),  //
        present_family_index.value()    //
    );
  }
};

static vk::SurfaceKHR CreateSurface(
    vk::Instance instance,
    VulkanConnection::SurfaceCallback surface_callback) {
  if (!surface_callback) {
    P_ERROR << "Surface callback was nullptr";
    return {};
  }

  auto surface = surface_callback(instance);

  if (!surface) {
    P_ERROR << "Could not create Window surface.";
    return {};
  }

  return surface;
}

static bool DeviceHasRequiredExtensions(const vk::PhysicalDevice& device) {
  auto device_extensions = device.enumerateDeviceExtensionProperties();
  if (device_extensions.result != vk::Result::eSuccess) {
    return false;
  }

  std::set<std::string> required_extensions(kRequiredDeviceExtensions.begin(),
                                            kRequiredDeviceExtensions.end());

  for (const auto& extension : device_extensions.value) {
    required_extensions.erase(
        static_cast<const char*>(extension.extensionName));
  }

  if (!required_extensions.empty()) {
    P_ERROR << "The following required device extensions were absent:";
    for (const auto& extension : required_extensions) {
      P_ERROR << extension;
    }
  }

  return required_extensions.empty();
}

static PhysicalDeviceSelection SelectPhysicalDevice(
    uint32_t device_index,
    const vk::PhysicalDevice& device,
    const vk::SurfaceKHR& surface) {
  const auto& queue_family_properties = device.getQueueFamilyProperties();
  for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
    PhysicalDeviceSelection selection;
    selection.device_index = device_index;

    // Check for graphics support.
    const auto& queue_family_property = queue_family_properties[i];
    if (!(queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics)) {
      continue;
    }

    selection.graphics_family_index = i;

    // Check for presentation support.
    const auto surface_supported = device.getSurfaceSupportKHR(i, surface);
    if (surface_supported.result != vk::Result::eSuccess) {
      continue;
    }

    if (!surface_supported.value) {
      continue;
    }

    selection.present_family_index = i;

    // Check for required extensions.
    if (!DeviceHasRequiredExtensions(device)) {
      continue;
    }

    selection.has_required_extensions = true;

    // Check for swapchain surface compatibility.
    SwapchainDetails swapchain_details;

    auto surface_capabilities = device.getSurfaceCapabilitiesKHR(surface);
    if (surface_capabilities.result != vk::Result::eSuccess) {
      continue;
    }

    swapchain_details.capabilities = surface_capabilities.value;

    auto surface_formats = device.getSurfaceFormatsKHR(surface);
    if (surface_formats.result != vk::Result::eSuccess) {
      continue;
    }

    swapchain_details.surface_formats = surface_formats.value;

    auto present_modes = device.getSurfacePresentModesKHR(surface);
    if (present_modes.result != vk::Result::eSuccess) {
      continue;
    }

    swapchain_details.present_modes = present_modes.value;

    selection.swapchain_details = std::move(swapchain_details);

    return selection;
  }

  return {};
}

static PhysicalDeviceSelection SelectPhysicalDevice(
    const std::vector<vk::PhysicalDevice>& devices,
    const vk::SurfaceKHR& surface) {
  for (uint32_t i = 0; i < devices.size(); i++) {
    if (auto selection = SelectPhysicalDevice(i, devices[i], surface)) {
      return selection;
    }
  }
  return {};
}

static constexpr bool AreValidationLayersEnabled() {
  return true;
}

static std::optional<std::set<std::string>> GetRequiredInstanceLayers() {
  std::set<std::string> required_layers;

  // Other required layers go here as necessary.

  if (AreValidationLayersEnabled()) {
    required_layers.insert("VK_LAYER_KHRONOS_validation");
  }

  const auto layer_properties = vk::enumerateInstanceLayerProperties();

  if (layer_properties.result != vk::Result::eSuccess) {
    P_ERROR << "Could not query instance layer properties.";
    return std::nullopt;
  }

  std::set<std::string> layers;
  for (const auto& layer : layer_properties.value) {
    std::string layer_name{layer.layerName};

    auto found = required_layers.find(layer_name);
    if (found == required_layers.end()) {
      continue;
    }
    layers.insert(*found);
    required_layers.erase(found);
  }

  if (required_layers.size() != 0u) {
    P_ERROR << "No all required layers were found. The ones missing were:";
    for (const auto& layer : required_layers) {
      P_ERROR << "Layer Name: " << layer;
    }
    return std::nullopt;
  }

  return layers;
}

static std::optional<std::set<std::string>> GetRequiredInstanceExtensions(
    std::set<std::string> required_extensions) {
  if (AreValidationLayersEnabled()) {
    required_extensions.insert(std::string{VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
  }

  const auto extension_properties = vk::enumerateInstanceExtensionProperties();
  if (extension_properties.result != vk::Result::eSuccess) {
    P_ERROR << "Could not query instance layer properties.";
    return std::nullopt;
  }

  std::set<std::string> extensions;
  for (const auto& extension_property : extension_properties.value) {
    std::string extension_name{extension_property.extensionName};
    auto found = required_extensions.find(extension_name);
    if (found == required_extensions.end()) {
      continue;
    }
    extensions.insert(*found);
    required_extensions.erase(found);
  }

  if (required_extensions.size() != 0u) {
    P_ERROR
        << "Not all required extensions were found. The ones missing were: ";
    for (const auto& required_extension : required_extensions) {
      P_ERROR << "Extension Name: " << required_extension;
    }
    return std::nullopt;
  }

  return extensions;
}

static vk::PhysicalDeviceFeatures GetEnabledFeatures(
    const vk::PhysicalDeviceFeatures& available_features) {
  vk::PhysicalDeviceFeatures enabled_features;

  if (available_features.samplerAnisotropy) {
    enabled_features.samplerAnisotropy = true;
  }

  return enabled_features;
}

VulkanConnection::VulkanConnection(
    PFN_vkGetInstanceProcAddr get_instance_proc_address,
    SurfaceCallback get_surface_proc_addr,
    std::set<std::string> required_instance_extensions) {
  if (!get_instance_proc_address) {
    P_ERROR << "Instance proc address accessor was not available.";
    return;
  }

  if (!get_surface_proc_addr) {
    P_ERROR << "Surface proc address accessor was not available.";
    return;
  }

  VULKAN_HPP_DEFAULT_DISPATCHER.init(get_instance_proc_address);

  vk::ApplicationInfo application_info;
  application_info.setPApplicationName("machine");
  application_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setPEngineName("machine");
  application_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setApiVersion(VK_MAKE_VERSION(1, 0, 0));

  const auto required_layers = GetRequiredInstanceLayers();
  const auto required_extensions =
      GetRequiredInstanceExtensions(required_instance_extensions);

  if (!required_layers.has_value() || !required_extensions.has_value()) {
    P_ERROR
        << "Could not consolidate the required Vulkan extensions or layers.";
    return;
  }

  std::vector<const char*> required_layers_vector;
  for (const auto& layer : required_layers.value()) {
    required_layers_vector.push_back(layer.c_str());
  }
  std::vector<const char*> required_extensions_vector;
  for (const auto& extension : required_extensions.value()) {
    required_extensions_vector.push_back(extension.c_str());
  }

  vk::InstanceCreateInfo instance_create_info;
  instance_create_info.setPApplicationInfo(&application_info);

  if (required_extensions_vector.size() != 0) {
    instance_create_info.setPpEnabledExtensionNames(
        required_extensions_vector.data());
    instance_create_info.setEnabledExtensionCount(
        required_extensions_vector.size());
  }

  if (required_layers_vector.size() != 0) {
    instance_create_info.setEnabledLayerCount(required_layers_vector.size());
    instance_create_info.setPpEnabledLayerNames(required_layers_vector.data());
  }

  auto instance_result = vk::createInstanceUnique(instance_create_info);

  if (instance_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create Vulkan instance.";
    return;
  }

  auto instance = std::move(instance_result.value);

  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

  // The debug messenger must be attached as soon as the instance is created so
  // that further calls are intercepted.
  vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger;
  if (AreValidationLayersEnabled()) {
    vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_info;
    using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    debug_utils_messenger_info.setMessageSeverity(
        Severity::eError | Severity::eWarning | Severity::eInfo |
        Severity::eVerbose);
    using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
    debug_utils_messenger_info.setMessageType(
        Type::eGeneral | Type::ePerformance | Type::eValidation);
    debug_utils_messenger_info.setPfnUserCallback(
        [](VkDebugUtilsMessageSeverityFlagBitsEXT severity,
           VkDebugUtilsMessageTypeFlagsEXT types,
           const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
           void* user_data) -> VkBool32 {
          return OnDebugUtilsMessengerCallback(
              static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity),
              static_cast<vk::DebugUtilsMessageTypeFlagBitsEXT>(types),
              {*callback_data});
        });
    auto debug_messenger_result =
        instance.get().createDebugUtilsMessengerEXTUnique(
            debug_utils_messenger_info);
    if (debug_messenger_result.result != vk::Result::eSuccess) {
      P_ERROR << "Validation was enabled but could not create debug utils "
                 "messenger.";
      return;
    }
    debug_utils_messenger = std::move(debug_messenger_result.value);
  }

  auto surface = CreateSurface(instance.get(), get_surface_proc_addr);

  if (!surface) {
    P_ERROR << "Could not create surface.";
    return;
  }

  auto physical_devices_result = instance->enumeratePhysicalDevices();

  if (physical_devices_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not enumerate physical devices.";
    return;
  }

  auto physical_devices = std::move(physical_devices_result.value);

  if (physical_devices.size() == 0) {
    P_ERROR << "Instance has no devices.";
    return;
  }

  auto selection = SelectPhysicalDevice(physical_devices, surface);

  if (!selection) {
    P_ERROR << "No suitable device available.";
    return;
  }

  vk::DeviceQueueCreateInfo queue_create_info;
  queue_create_info.setQueueFamilyIndex(
      selection.graphics_family_index.value());
  queue_create_info.setQueueCount(1u);
  const float queue_priority = 1.0f;
  queue_create_info.setPQueuePriorities(&queue_priority);

  auto enabled_features = GetEnabledFeatures(
      physical_devices[selection.device_index.value()].getFeatures());

  vk::DeviceCreateInfo device_create_info;
  device_create_info.setPQueueCreateInfos(&queue_create_info);
  device_create_info.setQueueCreateInfoCount(1u);
  device_create_info.setPpEnabledExtensionNames(
      kRequiredDeviceExtensions.data());
  device_create_info.setEnabledExtensionCount(kRequiredDeviceExtensions.size());
  device_create_info.setPEnabledFeatures(&enabled_features);

  auto device_result =
      physical_devices[selection.device_index.value()].createDeviceUnique(
          device_create_info);
  if (device_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create logical device.";
    return;
  }

  auto device = std::move(device_result.value);

  VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

  auto swapchain = selection.CreateSwapchain(*this, device.get(), surface);
  if (!swapchain) {
    P_ERROR << "Could not create swapchain.";
    return;
  }

  instance_ = std::move(instance);
  physical_device_selection_ =
      std::make_unique<PhysicalDeviceSelection>(selection);
  physical_device_ = physical_devices[selection.device_index.value()];
  device_ = std::move(device);
  surface_ = std::move(surface);
  swapchain_ = std::move(swapchain);
  debug_utils_messenger_ = std::move(debug_utils_messenger);
  available_features_ = enabled_features;

  is_valid_ = true;
}

VulkanConnection::~VulkanConnection() {
  swapchain_.reset();
  if (instance_.get() && surface_) {
    instance_.get().destroySurfaceKHR(surface_);
  }
}

bool VulkanConnection::IsValid() const {
  return is_valid_;
}

vk::Instance VulkanConnection::GetInstance() const {
  return instance_.get();
}

vk::Device VulkanConnection::GetDevice() const {
  return device_.get();
}

bool VulkanConnection::OnDebugUtilsMessengerCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT types,
    const vk::DebugUtilsMessengerCallbackDataEXT& callback_data) {
  if (!(severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)) {
    return false;
  }

  P_ERROR << "[Vulkan Validation] " << to_string(severity)
          << callback_data.pMessage;

  return false;
}

VulkanSwapchain& VulkanConnection::GetSwapchain() const {
  return *swapchain_.get();
}

uint32_t VulkanConnection::GetGraphicsQueueFamilyIndex() const {
  return physical_device_selection_->graphics_family_index.value();
}

const vk::PhysicalDeviceFeatures& VulkanConnection::GetAvailableFeatures()
    const {
  return available_features_;
}

vk::PhysicalDevice VulkanConnection::GetPhysicalDevice() const {
  return physical_device_;
}

std::shared_ptr<RenderingContext> VulkanConnection::CreateRenderingContext()
    const {
  if (!IsValid()) {
    return nullptr;
  }

  auto family_index = physical_device_selection_->graphics_family_index.value();
  auto queue = device_.get().getQueue(family_index, 0u);

  // TODO: Both graphics and transfer queues are same. Separate them out.
  QueueSelection graphics_queue = {family_index, queue};
  QueueSelection transfer_queue = {family_index, queue};

  auto context =
      std::make_shared<RenderingContext>(*this,               // delegate
                                         instance_.get(),     // instance
                                         physical_device_,    // physical device
                                         device_.get(),       // logical device
                                         graphics_queue,      // graphics queue
                                         transfer_queue,      // transfer queue
                                         available_features_  // features
      );

  if (!context->IsValid()) {
    return nullptr;
  }

  return context;
}

// |VulkanSwapchain::Delegate|
void VulkanConnection::OnSwapchainNeedsRecreation(
    const VulkanSwapchain& caller) {
  if (!swapchain_) {
    return;
  }

  if (swapchain_.get() != &caller) {
    return;
  }

  auto new_capabilities = physical_device_.getSurfaceCapabilitiesKHR(surface_);

  if (new_capabilities.result != vk::Result::eSuccess) {
    P_ERROR << "Could not query new surface capabilities while attempting to "
               "recreate sub-optimal swapchain.";
    return;
  }

  const auto new_extents =
      VulkanSwapchain::ChooseSwapExtents(new_capabilities.value);

  auto old_swapchain = std::move(swapchain_);

  old_swapchain->Retire();

  auto new_swapchain =
      std::make_unique<VulkanSwapchain>(*old_swapchain, new_extents);

  EventLoop::ForCurrentThread().GetDispatcher()->PostTask(MakeCopyable(
      [swapchain = std::move(old_swapchain)]() mutable { swapchain.reset(); }));

  if (!new_swapchain->IsValid()) {
    return;
  }

  swapchain_ = std::move(new_swapchain);
}

// |RenderingContext::Delegate|
vk::RenderPass VulkanConnection::GetOnScreenRenderPass() const {
  return swapchain_ ? swapchain_->GetRenderPass() : vk::RenderPass{};
}

// |RenderingContext::Delegate|
size_t VulkanConnection::GetSwapchainImageCount() const {
  return swapchain_ ? swapchain_->GetImageCount() : 0u;
}

// |RenderingContext::Delegate|
vk::Extent2D VulkanConnection::GetScreenExtents() const {
  return swapchain_ ? swapchain_->GetExtents() : vk::Extent2D{};
}

}  // namespace pixel
