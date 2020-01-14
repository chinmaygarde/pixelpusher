#pragma once

#include "macros.h"
#include "vulkan.h"
#include "vulkan_connection.h"

namespace pixel {

class Renderer {
 public:
  Renderer(GLFWwindow* window);

  ~Renderer();

  bool IsValid() const;

  void Setup();

  bool Render();

  void Teardown();

 private:
  VulkanConnection connection_;

  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(Renderer);
};

}  // namespace pixel
