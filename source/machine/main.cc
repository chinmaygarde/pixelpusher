#define GLFW_INCLUDE_VULKAN

#include <cstdlib>
#include <iostream>

#include "closure.h"
#include "logging.h"
#include "renderer.h"
#include "vulkan.h"

namespace pixel {

void OnGLFWError(int error_code, const char* description) {
  P_ERROR << "GLFW Error: " << description << "(" << error_code << ")";
}

int Main(int argc, char const* argv[]) {
  if (!glfwInit()) {
    P_ERROR << "GLFW could not be initialized.";
    return EXIT_FAILURE;
  }

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

  Renderer renderer(window);

  if (!renderer.IsValid()) {
    P_ERROR << "Could not create a valid renderer.";
    return EXIT_FAILURE;
  }

  if (!renderer.Setup()) {
    P_ERROR << "Could not setup renderer.";
    return EXIT_FAILURE;
  }

  AutoClosure teardown_renderer([&renderer]() { renderer.Teardown(); });

  while (!glfwWindowShouldClose(window)) {
    if (!renderer.Render()) {
      P_ERROR << "Error while attempting to render.";
      return EXIT_FAILURE;
    }
    glfwWaitEvents();
  }

  return EXIT_SUCCESS;
}

}  // namespace pixel

int main(int argc, char const* argv[]) {
  return pixel::Main(argc, argv);
}
