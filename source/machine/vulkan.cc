#include "vulkan.h"

#include <optional>

#include "macros.h"
#include "logging.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace pixel {

bool InitVulkan(GLFWwindow* glfw_window) {
  if (glfwVulkanSupported() != GLFW_TRUE) {
    P_ERROR << "GLFW could not setup Vulkan.";
    return false;
  }
  
  auto get_instance_proc_address = reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress(nullptr, "vkGetInstanceProcAddr"));
  VULKAN_HPP_DEFAULT_DISPATCHER.init(get_instance_proc_address);

  vk::ApplicationInfo application_info;
  application_info.setPApplicationName("machine");
  application_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setPEngineName("machine");
  application_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setApiVersion(VK_MAKE_VERSION(1, 0, 0));
  
  uint32_t required_extensions_count = 0;
  const char** required_extensions = glfwGetRequiredInstanceExtensions(&required_extensions_count);
  
  vk::InstanceCreateInfo instance_create_info;
  instance_create_info.setPpEnabledExtensionNames(required_extensions);
  instance_create_info.setEnabledExtensionCount(required_extensions_count);
  instance_create_info.setPApplicationInfo(&application_info);
  
  auto [r1, instance] = vk::createInstanceUnique(instance_create_info);
  
  if (!instance.get()) {
    P_ERROR << "Could not create Vulkan instance.";
    return false;
  }
  
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());
  
  auto [r2, physical_devices] = instance->enumeratePhysicalDevices();
  if (physical_devices.size() == 0) {
    P_ERROR << "Instance has no devices.";
    return false;
  }
  
  const auto& physical_device = physical_devices[0];
    
  std::optional<uint32_t> queue_family_index;
  const auto& queue_family_properties = physical_device.getQueueFamilyProperties();
  for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
    const auto& queue_family_property = queue_family_properties[i];
    if (queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics) {
      queue_family_index = i;
    }
  }
  
  if (!queue_family_index.has_value()) {
    P_ERROR << "Selected device was not graphics capable.";
    return false;
  }
  
  vk::DeviceQueueCreateInfo queue_create_info;
  queue_create_info.setQueueFamilyIndex(queue_family_index.value());
  
  vk::DeviceCreateInfo device_create_info;
  device_create_info.setPQueueCreateInfos(&queue_create_info);
  device_create_info.setQueueCreateInfoCount(1u);
  
  auto [r3, device] = physical_devices[0].createDeviceUnique(device_create_info);
  if (!device.get()) {
    P_ERROR << "Could not create logical device.";
    return false;
  }
  
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());
  
  VkSurfaceKHR vk_surface = {};
  
  if (glfwCreateWindowSurface(static_cast<VkInstance>(instance.get()), glfw_window, nullptr, &vk_surface) != VK_SUCCESS) {
    P_ERROR << "Could not create Vulkan Surface";
    return false;
  }
  
  vk::UniqueSurfaceKHR surface(vk_surface);
  
  if (!surface.get()) {
    P_ERROR << "Could not create Window surface.";
    return false;
  }
  
  return true;
}

}  // namespace pixel
