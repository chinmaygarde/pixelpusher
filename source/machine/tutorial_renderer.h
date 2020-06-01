#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "image_decoder.h"
#include "macros.h"
#include "memory_allocator.h"
#include "renderer.h"
#include "shaders/triangle.h"
#include "vulkan.h"

namespace pixel {

class TutorialRenderer final : public Renderer {
 public:
  TutorialRenderer(std::shared_ptr<RenderingContext> context);

  // |Renderer|
  ~TutorialRenderer() override;

  // |Renderer|
  bool IsValid() const override;

  // |Renderer|
  bool Setup() override;

  // |Renderer|
  bool BeginFrame() override;

  // |Renderer|
  bool RenderFrame(vk::CommandBuffer render_command_buffer) override;

  // |Renderer|
  bool Teardown() override;

 private:
  using Clock = std::chrono::high_resolution_clock;

  vk::Device device_;
  vk::UniqueDescriptorSetLayout descriptor_set_layout_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> index_buffer_;
  std::unique_ptr<ImageView> image_;
  vk::UniquePipeline pipeline_;
  UniformBuffer<TriangleUBO> triangle_ubo_;
  vk::UniquePipelineLayout pipeline_layout_;
  std::vector<vk::DescriptorSet> descriptor_sets_;
  vk::UniqueSampler sampler_;

  float fov_ = 90.0f;
  float rotation_rate_ = 10.0f;

  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(TutorialRenderer);
};

}  // namespace pixel
