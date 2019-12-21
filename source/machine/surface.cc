#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES

#include "surface.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace pixel {

Surface::Surface() {
  vk::ApplicationInfo application_info;
  application_info.setPApplicationName("machine");
  application_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setPEngineName("machine");
  application_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
  application_info.setApiVersion(VK_MAKE_VERSION(1, 0, 0));

  vk::InstanceCreateInfo instance_create_info;

  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddrPtr =
      reinterpret_cast<PFN_vkGetInstanceProcAddr>(
          glfwGetInstanceProcAddress(nullptr, "vkGetInstanceProcAddr"));

  vk::DispatchLoaderDynamic loader(vkGetInstanceProcAddrPtr);

  vk::UniqueInstance instance;
}

Surface::~Surface() {}

}  // namespace pixel
