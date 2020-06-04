#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "image.h"
#include "image_decoder.h"
#include "macros.h"
#include "memory_allocator.h"
#include "renderer.h"
#include "shader_library.h"
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
  ShaderLibrary shader_library_;
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
  float rotation_rate_ = 100.0f;
  float z_near_ = 0.001f;
  float z_far_ = 10.0f;
  float eye_[3] = {1.0, 1.0, 1.0};

  bool is_valid_ = false;

  bool RebuildPipelines();

  P_DISALLOW_COPY_AND_ASSIGN(TutorialRenderer);
};

}  // namespace pixel
