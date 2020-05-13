#pragma once

#include <chrono>
#include <memory>

#include "command_pool.h"
#include "image_decoder.h"
#include "macros.h"
#include "memory_allocator.h"
#include "shaders/triangle.h"
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
  using Clock = std::chrono::high_resolution_clock;

  VulkanConnection connection_;
  vk::Device device_;
  std::shared_ptr<CommandPool> command_pool_;
  vk::UniqueDescriptorSetLayout descriptor_set_layout_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> index_buffer_;
  std::unique_ptr<ImageView> image_;
  vk::UniquePipeline pipeline_;
  UniformBuffer<TriangleUBO> triangle_ubo_;
  vk::UniquePipelineLayout pipeline_layout_;
  vk::UniqueDescriptorPool descriptor_pool_;
  std::vector<vk::DescriptorSet> descriptor_sets_;

  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(Renderer);
};

}  // namespace pixel
