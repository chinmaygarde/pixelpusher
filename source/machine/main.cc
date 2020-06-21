#include <stdlib.h>

#include <cstdlib>
#include <iostream>

#include "closure.h"
#include "event_loop.h"
#include "geometry.h"
#include "glfw.h"
#include "key_input_glfw.h"
#include "logging.h"
#include "main_renderer.h"
#include "platform.h"
#include "renderer.h"
#include "vulkan.h"
#include "vulkan_connection.h"

namespace pixel {

static void OnGLFWError(int error_code, const char* description) {
  P_ERROR << "GLFW Error: " << description << "(" << error_code << ")";
}

static std::set<std::string> GetGLFWRequiredInstanceExtensions() {
  uint32_t count = 0;
  const char** c_extensions = ::glfwGetRequiredInstanceExtensions(&count);
  std::set<std::string> extensions;
  for (size_t i = 0; i < count; i++) {
    extensions.insert(std::string{c_extensions[i]});
  }
  return extensions;
}

static Size GetCurrentWindowSize(GLFWwindow* window) {
  int width = 0;
  int height = 0;
  ::glfwGetFramebufferSize(window, &width, &height);
  if (width < 0) {
    width = 0;
  }
  if (height < 0) {
    height = 0;
  }
  return {static_cast<size_t>(width), static_cast<size_t>(height)};
}

struct GLFWDelegate {
  KeyInputDispatcher key_input_dispatcher;
};

static void OnGLFWKeyEvent(GLFWwindow* window,
                           int key,
                           int scancode,
                           int action,
                           int mods) {
  auto delegate =
      reinterpret_cast<GLFWDelegate*>(::glfwGetWindowUserPointer(window));

  if (!delegate) {
    return;
  }

  delegate->key_input_dispatcher.DispatchKey(GLFWKeyTypeToKeyType(key),
                                             GLFWKeyActionToAction(action),
                                             GLFWKeyModifiersToModifiers(mods)

  );
}

static bool Main(int argc, char const* argv[]) {
  if (!::glfwInit()) {
    P_ERROR << "GLFW could not be initialized.";
    return false;
  }

  if (::glfwVulkanSupported() != GLFW_TRUE) {
    P_ERROR << "Vulkan support unavailable.";
    return false;
  }

  ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  AutoClosure glfw_terminate([]() { ::glfwTerminate(); });

  ::glfwSetErrorCallback(&OnGLFWError);

  auto window = ::glfwCreateWindow(800, 600, "Machine", NULL, NULL);

  if (!window) {
    P_ERROR << "Could not create GLFW window.";
    return false;
  }

  AutoClosure destroy_window([window]() { ::glfwDestroyWindow(window); });

  GLFWDelegate glfw_delegate;

  ::glfwSetWindowUserPointer(window, &glfw_delegate);
  ::glfwSetKeyCallback(window, &OnGLFWKeyEvent);

  auto get_instance_proc_address = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      ::glfwGetInstanceProcAddress(nullptr, "vkGetInstanceProcAddr"));
  auto surface_callback = [window](vk::Instance instance) -> vk::SurfaceKHR {
    VkSurfaceKHR vk_surface = {};

    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance),  // instance
                                window,                             // window
                                nullptr,     // allocation callbacks
                                &vk_surface  // surface (out)
                                ) != VK_SUCCESS) {
      P_ERROR << "Could not create window surface.";
      return {};
    }

    return {vk_surface};
  };

  VulkanConnection connection(
      get_instance_proc_address,           // instance proc addr
      surface_callback,                    // surface proc addr
      GetGLFWRequiredInstanceExtensions()  // instance extensions
  );

  if (!connection.IsValid()) {
    return false;
  }

  MainRenderer renderer(connection, window);

  if (!renderer.IsValid()) {
    P_ERROR << "Could not create a valid renderer.";
    return false;
  }

  if (!renderer.Setup()) {
    P_ERROR << "Could not setup renderer.";
    return false;
  }

  glfw_delegate.key_input_dispatcher.AddDelegate(&renderer);

  AutoClosure teardown_renderer([&renderer]() { renderer.Teardown(); });

  auto& loop = EventLoop::ForCurrentThread();

  while (true) {
    if (!loop.FlushTasksNow()) {
      P_ERROR << "Could not flush event loop tasks.";
      return false;
    }

    ::glfwPollEvents();

    if (::glfwWindowShouldClose(window)) {
      break;
    }

    const auto window_size = GetCurrentWindowSize(window);
    if (window_size.width == 0 || window_size.height == 0) {
      ::glfwWaitEvents();
      continue;
    }

    if (!renderer.Render()) {
      P_ERROR << "Error while attempting to render.";
      return false;
    }
  }

  glfw_delegate.key_input_dispatcher.RemoveDelegate(&renderer);

  return true;
}

}  // namespace pixel

int main(int argc, char const* argv[]) {
  const auto return_code =
      pixel::Main(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
#if P_OS_WIN
  ::ExitProcess(return_code);
#endif  // P_OS_WIN
  return return_code;
}
