#pragma once

#include <memory>

#include "command_pool.h"
#include "macros.h"
#include "vulkan.h"
#include "vulkan_connection.h"

namespace pixel {

class Renderer {
 public:
  Renderer(GLFWwindow* window);

  ~Renderer();

  bool IsValid() const;

  bool Setup();

  bool Render();

  bool Teardown();

 private:
  VulkanConnection connection_;
  std::shared_ptr<CommandPool> command_pool_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> index_buffer_;
  vk::UniquePipeline pipeline_;

  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(Renderer);
};

}  // namespace pixel
