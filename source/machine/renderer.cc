#include "renderer.h"

#include "logging.h"

namespace pixel {

Renderer::Renderer(GLFWwindow* window) : connection_(window) {
  if (!connection_.IsValid()) {
    P_ERROR << "Vulkan connection was invalid.";
    return;
  }

  is_valid_ = true;
}

Renderer::~Renderer() = default;

bool Renderer::IsValid() const {
  return is_valid_;
}

void Renderer::Setup() {}

bool Renderer::Render() {
  return true;
}

void Renderer::Teardown() {}

}  // namespace pixel
