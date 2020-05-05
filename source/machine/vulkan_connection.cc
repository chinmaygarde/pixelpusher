#include "vulkan_connection.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <set>
#include <string>

#include "logging.h"
#include "macros.h"
#include "vulkan_swapchain.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace pixel {

static const std::vector<const char*> kRequiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

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

  std::optional<vk::Extent2D> ChooseSwapExtent() const {
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

  uint32_t ChooseSwapchainImageCount() const {
    return std::clamp<uint32_t>(3, capabilities.minImageCount,
                                capabilities.maxImageCount);
  };

  std::unique_ptr<VulkanSwapchain> CreateSwapchain(
      const vk::Device& device,
      const vk::SurfaceKHR& surface,
      uint32_t graphics_family_index,
      uint32_t present_family_index) const {
    auto surface_format = ChooseSurfaceFormat();
    auto present_mode = ChoosePresentMode();
    auto swap_extent = ChooseSwapExtent();

    if (!surface_format || !present_mode || !swap_extent) {
      return nullptr;
    }

    vk::SwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info.setSurface(surface);
    swapchain_create_info.setMinImageCount(ChooseSwapchainImageCount());
    swapchain_create_info.setImageFormat(surface_format.value().format);
    swapchain_create_info.setImageColorSpace(surface_format.value().colorSpace);
    swapchain_create_info.setPresentMode(present_mode.value());
    swapchain_create_info.setImageExtent(swap_extent.value());
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

    auto swapchain = device.createSwapchainKHRUnique(swapchain_create_info);
    if (swapchain.result != vk::Result::eSuccess) {
      return nullptr;
    }

    auto vulkan_swapchain =
        std::make_unique<VulkanSwapchain>(device,                         //
                                          std::move(swapchain.value),     //
                                          surface_format.value().format,  //
                                          graphics_family_index,          //
                                          swap_extent.value()             //
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
      const vk::Device& device,
      const vk::SurfaceKHR& surface) {
    if (!IsValid()) {
      return nullptr;
    }

    return swapchain_details.value().CreateSwapchain(
        device, surface, graphics_family_index.value(),
        present_family_index.value());
  }
};

static vk::SurfaceKHR CreateSurface(const vk::Instance& instance,
                                    GLFWwindow* window) {
  VkSurfaceKHR vk_surface = {};

  if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window,
                              nullptr, &vk_surface) != VK_SUCCESS) {
    P_ERROR << "Could not create Vulkan Surface";
    return {};
  }

  vk::SurfaceKHR surface(vk_surface);

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

static std::optional<std::set<std::string>> GetRequiredInstanceExtensions() {
  uint32_t glfw_extensions_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

  std::set<std::string> required_extensions;

  for (size_t i = 0; i < glfw_extensions_count; i++) {
    required_extensions.insert(std::string{glfw_extensions[i]});
  }

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

VulkanConnection::VulkanConnection(GLFWwindow* glfw_window) {
  if (glfw_window == nullptr) {
    P_ERROR << "GLFW window invalid.";
    return;
  }

  if (glfwVulkanSupported() != GLFW_TRUE) {
    P_ERROR << "GLFW could not setup Vulkan.";
    return;
  }

  auto get_instance_proc_address = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      glfwGetInstanceProcAddress(nullptr, "vkGetInstanceProcAddr"));
  VULKAN_HPP_DEFAULT_DISPATCHER.init(get_instance_proc_address);

  vk::ApplicationInfo application_info;
  application_info.setPApplicationName("machine");
  application_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setPEngineName("machine");
  application_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setApiVersion(VK_MAKE_VERSION(1, 0, 0));

  const auto required_layers = GetRequiredInstanceLayers();
  const auto required_extensions = GetRequiredInstanceExtensions();

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

  auto surface = CreateSurface(instance.get(), glfw_window);

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

  vk::DeviceCreateInfo device_create_info;
  device_create_info.setPQueueCreateInfos(&queue_create_info);
  device_create_info.setQueueCreateInfoCount(1u);
  device_create_info.setPpEnabledExtensionNames(
      kRequiredDeviceExtensions.data());
  device_create_info.setEnabledExtensionCount(kRequiredDeviceExtensions.size());

  auto device_result =
      physical_devices[selection.device_index.value()].createDeviceUnique(
          device_create_info);
  if (device_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create logical device.";
    return;
  }

  auto device = std::move(device_result.value);

  VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

  auto swapchain = selection.CreateSwapchain(device.get(), surface);
  if (!swapchain) {
    P_ERROR << "Could not create swapchain.";
    return;
  }

  auto memory_allocator = std::make_unique<MemoryAllocator>(
      physical_devices[selection.device_index.value()], device);
  if (!memory_allocator->IsValid()) {
    P_ERROR << "Could not create device memory allocator.";
    return;
  }

  instance_ = std::move(instance);
  physical_device_selection_ =
      std::make_unique<PhysicalDeviceSelection>(selection);
  device_ = std::move(device);
  surface_ = std::move(surface);
  swapchain_ = std::move(swapchain);
  memory_allocator_ = std::move(memory_allocator);
  debug_utils_messenger_ = std::move(debug_utils_messenger);

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

const vk::Device& VulkanConnection::GetDevice() const {
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
  P_ASSERT(is_valid_);
  return *swapchain_.get();
}

MemoryAllocator& VulkanConnection::GetMemoryAllocator() const {
  P_ASSERT(is_valid_);
  return *memory_allocator_.get();
}

uint32_t VulkanConnection::GetGraphicsQueueFamilyIndex() const {
  P_ASSERT(is_valid_);
  return physical_device_selection_->graphics_family_index.value();
}

}  // namespace pixel
