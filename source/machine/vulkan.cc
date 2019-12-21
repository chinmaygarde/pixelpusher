#include "vulkan.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace pixel {

bool InitVulkan() {
  if (glfwVulkanSupported() != GLFW_TRUE) {
    return false;
  }

  vk::DynamicLoader step1_loader;
  auto get_proc_address =
      step1_loader.getProcAddress<PFN_vkGetInstanceProcAddr>(
          "vkGetInstanceProcAddr");
}

}  // namespace pixel
