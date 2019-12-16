#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>

void OnGLFWError(int error_code, const char* description) {
  std::cerr << "GLFW Error: " << description << "(" << error_code << ")"
            << std::endl;
}

int main(int argc, char const* argv[]) {
  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW." << std::endl;
    return EXIT_FAILURE;
  }

  glfwSetErrorCallback(&OnGLFWError);

  if (glfwVulkanSupported() != GLFW_TRUE) {
    std::cerr << "GLFW does not support Vulkan." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
