#include "vulkan_connection.h"

#include <optional>

#include "logging.h"
#include "macros.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace pixel {

struct DeviceSelection {
  std::optional<uint32_t> device_index;
  std::optional<uint32_t> queue_family_index;
  std::optional<uint32_t> present_family_index;

  operator bool() const {
    return queue_family_index.has_value() && present_family_index.has_value() &&
           device_index.has_value();
  }
};

static vk::UniqueSurfaceKHR CreateSurface(const vk::Instance& instance,
                                          GLFWwindow* window) {
  VkSurfaceKHR vk_surface = {};

  if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window,
                              nullptr, &vk_surface) != VK_SUCCESS) {
    P_ERROR << "Could not create Vulkan Surface";
    return {};
  }

  vk::UniqueSurfaceKHR surface(vk_surface);

  if (!surface.get()) {
    P_ERROR << "Could not create Window surface.";
    return {};
  }

  return surface;
}

static DeviceSelection IsDeviceSuitable(uint32_t device_index,
                                        const vk::PhysicalDevice& device,
                                        const vk::SurfaceKHR& surface) {
  DeviceSelection selection;
  selection.device_index = device_index;
  const auto& queue_family_properties = device.getQueueFamilyProperties();
  for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
    const auto& queue_family_property = queue_family_properties[i];
    if (!(queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics)) {
      continue;
    }

    selection.queue_family_index = i;

    const auto surface_supported = device.getSurfaceSupportKHR(i, surface);
    if (surface_supported.result != vk::Result::eSuccess) {
      continue;
    }

    if (!surface_supported.value) {
      continue;
    }

    selection.present_family_index = surface_supported.value;
    break;
  }
  return selection;
}

static DeviceSelection PickSuitableDevice(
    const std::vector<vk::PhysicalDevice>& devices,
    const vk::SurfaceKHR& surface) {
  for (uint32_t i = 0; i < devices.size(); i++) {
    if (auto selection = IsDeviceSuitable(i, devices[i], surface)) {
      return selection;
    }
  }
  return {};
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

  uint32_t required_extensions_count = 0;
  const char** required_extensions =
      glfwGetRequiredInstanceExtensions(&required_extensions_count);

  vk::InstanceCreateInfo instance_create_info;
  instance_create_info.setPpEnabledExtensionNames(required_extensions);
  instance_create_info.setEnabledExtensionCount(required_extensions_count);
  instance_create_info.setPApplicationInfo(&application_info);

  auto [r1, instance] = vk::createInstanceUnique(instance_create_info);

  if (!instance.get()) {
    P_ERROR << "Could not create Vulkan instance.";
    return;
  }

  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

  auto surface = CreateSurface(instance.get(), glfw_window);

  if (!surface) {
    P_ERROR << "Could not create surface.";
    return;
  }

  auto [r2, physical_devices] = instance->enumeratePhysicalDevices();
  if (physical_devices.size() == 0) {
    P_ERROR << "Instance has no devices.";
    return;
  }

  auto selection = PickSuitableDevice(physical_devices, surface.get());

  if (!selection) {
    P_ERROR << "No suitable device available.";
    return;
  }

  vk::DeviceQueueCreateInfo queue_create_info;
  queue_create_info.setQueueFamilyIndex(selection.queue_family_index.value());

  vk::DeviceCreateInfo device_create_info;
  device_create_info.setPQueueCreateInfos(&queue_create_info);
  device_create_info.setQueueCreateInfoCount(1u);

  auto [r3, device] =
      physical_devices[selection.device_index.value()].createDeviceUnique(
          device_create_info);
  if (!device.get()) {
    P_ERROR << "Could not create logical device.";
    return;
  }

  VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

  instance_ = std::move(instance);
  surface_ = std::move(surface);
  device_ = std::move(device);
  is_valid_ = true;
}

VulkanConnection::~VulkanConnection() = default;

bool VulkanConnection::IsValid() const {
  return is_valid_;
}

}  // namespace pixel
