#include "vulkan.h"

#include "macros.h"
#include "logging.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace pixel {

bool InitVulkan() {
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
  
  vk::DeviceCreateInfo device_create_info;
  
  auto [r3, device] = physical_devices[0].createDeviceUnique(device_create_info);
  if (!device.get()) {
    P_ERROR << "Could not create logical device.";
    return false;
  }
  
  return true;
}

}  // namespace pixel
