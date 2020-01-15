#include "renderer.h"

#include "logging.h"
#include "shader_loader.h"

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

bool Renderer::Setup() {
  auto vertex_shader = LoadShaderModule(connection_.GetDevice(), "triangle.vert");
  auto fragment_shader = LoadShaderModule(connection_.GetDevice(), "triangle.frag");
  
  if (!vertex_shader || !fragment_shader) {
    P_ERROR << "Could not load shader modules.";
    return false;
  }
  
  return true;
}

bool Renderer::Render() {
  return true;
}

bool Renderer::Teardown() {
  return true;
}

}  // namespace pixel
