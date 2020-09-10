#include <stdlib.h>

#include <cstdlib>
#include <iostream>
#include <optional>

#include "closure.h"
#include "event_loop.h"
#include "geometry.h"
#include "glfw.h"
#include "key_input_glfw.h"
#include "logging.h"
#include "main_renderer.h"
#include "platform.h"
#include "pointer_input.h"
#include "renderer.h"
#include "runtime.h"
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
  PointerInputDispatcher pointer_input_dispatcher;
  std::optional<int> active_pointer_button;
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

static void OnGLFWMouseButton(GLFWwindow* window,
                              int button,
                              int action,
                              int mods) {
  auto delegate =
      reinterpret_cast<GLFWDelegate*>(::glfwGetWindowUserPointer(window));

  if (!delegate) {
    return;
  }

  bool add_or_remove = true;
  switch (action) {
    case GLFW_PRESS:
      add_or_remove = true;
      break;
    case GLFW_RELEASE:
      add_or_remove = false;
      break;
    default:
      return;
  }

  if (add_or_remove) {
    if (delegate->active_pointer_button.has_value()) {
      delegate->pointer_input_dispatcher.StopTrackingPointer(
          delegate->active_pointer_button.value(), true);
    }
    delegate->active_pointer_button = button;
    delegate->pointer_input_dispatcher.StartTrackingPointer(button);
  } else {
    delegate->active_pointer_button = std::nullopt;
    delegate->pointer_input_dispatcher.StopTrackingPointer(button, false);
  }
}

static void OnGLFWCursorPosition(GLFWwindow* window, double x, double y) {
  auto delegate =
      reinterpret_cast<GLFWDelegate*>(::glfwGetWindowUserPointer(window));

  if (!delegate) {
    return;
  }

  if (!delegate->active_pointer_button.has_value()) {
    return;
  }

  delegate->pointer_input_dispatcher.DispatchPointerEvent(
      delegate->active_pointer_button.value(),
      Point{static_cast<int64_t>(x), static_cast<int64_t>(y)});
}

static bool Main(int argc, char const* argv[]) {
  RuntimeArgs runtime_args;
  runtime_args.SetAssetsPath();
  runtime_args.SetCommandLineArgs(argc, argv);
  Runtime runtime(runtime_args);
  if (!runtime.IsValid()) {
    P_ERROR << "Could not initialize the runtime.";
    return false;
  }

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
  ::glfwSetCursorPosCallback(window, &OnGLFWCursorPosition);
  ::glfwSetMouseButtonCallback(window, &OnGLFWMouseButton);

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
  glfw_delegate.pointer_input_dispatcher.AddDelegate(&renderer);

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
