#include <stdlib.h>

#include <cstdlib>
#include <iostream>

#include "closure.h"
#include "event_loop.h"
#include "logging.h"
#include "main_renderer.h"
#include "renderer.h"
#include "vulkan.h"
#include "vulkan_connection.h"

namespace pixel {

void OnGLFWError(int error_code, const char* description) {
  P_ERROR << "GLFW Error: " << description << "(" << error_code << ")";
}

int Main(int argc, char const* argv[]) {
  if (!glfwInit()) {
    P_ERROR << "GLFW could not be initialized.";
    return EXIT_FAILURE;
  }

  EventLoop::SetMainDispatcher(EventLoop::ForCurrentThread().GetDispatcher());

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  AutoClosure glfw_terminate([]() { glfwTerminate(); });

  glfwSetErrorCallback(&OnGLFWError);
  glfwWindowHint(GLFW_RESIZABLE, false);

  auto window = glfwCreateWindow(800, 600, "Machine", NULL, NULL);

  if (!window) {
    P_ERROR << "Could not create GLFW window.";
    return EXIT_FAILURE;
  }

  AutoClosure destroy_window([window]() { glfwDestroyWindow(window); });

  VulkanConnection connection(window);

  if (!connection.IsValid()) {
    return EXIT_FAILURE;
  }

  auto rendering_context = connection.CreateRenderingContext();

  if (!rendering_context || !rendering_context->IsValid()) {
    return EXIT_FAILURE;
  }

  MainRenderer renderer(connection, rendering_context, window);

  if (!renderer.IsValid()) {
    P_ERROR << "Could not create a valid renderer.";
    return EXIT_FAILURE;
  }

  if (!renderer.Setup()) {
    P_ERROR << "Could not setup renderer.";
    return EXIT_FAILURE;
  }

  AutoClosure teardown_renderer([&renderer]() { renderer.Teardown(); });

  auto& loop = EventLoop::ForCurrentThread();

  while (true) {
    if (!loop.FlushTasksNow()) {
      P_ERROR << "Could not flush event loop tasks.";
      return EXIT_FAILURE;
    }

    glfwPollEvents();

    if (glfwWindowShouldClose(window)) {
      break;
    }

    if (!renderer.Render()) {
      P_ERROR << "Error while attempting to render.";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

}  // namespace pixel

int main(int argc, char const* argv[]) {
  return pixel::Main(argc, argv);
}
