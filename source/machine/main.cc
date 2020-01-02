#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>

#include "auto_closure.h"
#include "logging.h"
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

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  AutoClosure glfw_terminate([]() { glfwTerminate(); });

  glfwSetErrorCallback(&OnGLFWError);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  auto window = glfwCreateWindow(800, 600, "Machine", NULL, NULL);

  if (!window) {
    P_ERROR << "Could not create GLFW window.";
    return EXIT_FAILURE;
  }

  auto vulkan_connection = std::make_unique<VulkanConnection>(window);

  if (!vulkan_connection->IsValid()) {
    P_ERROR << "Vulkan could not be initialized.";
    return EXIT_FAILURE;
  }

  AutoClosure destroy_window([window]() { glfwDestroyWindow(window); });

  while (!glfwWindowShouldClose(window)) {
    glfwWaitEvents();
  }

  return EXIT_SUCCESS;
}

}  // namespace pixel

int main(int argc, char const* argv[]) {
  return pixel::Main(argc, argv);
}
