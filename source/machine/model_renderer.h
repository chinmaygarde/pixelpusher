#pragma once

#include "asset_loader.h"
#include "macros.h"
#include "model.h"
#include "renderer.h"
#include "shader_library.h"
#include "shaders/model_renderer.h"
#include "vulkan.h"

namespace pixel {

class ModelRenderer : public Renderer {
 public:
  ModelRenderer(std::shared_ptr<RenderingContext> context,
                std::string model_assets_dir,
                std::string model_path);

  // |Renderer|
  ~ModelRenderer() override;

 private:
  std::unique_ptr<model::Model> model_;
  vk::UniqueDescriptorSetLayout descriptor_set_layout_;
  vk::UniquePipelineLayout pipeline_layout_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> index_buffer_;
  UniformBuffer<shaders::model_renderer::UniformBuffer> uniform_buffer_;
  DescriptorSets descriptor_sets_;
  vk::UniquePipeline pipeline_;
  bool is_valid_ = false;

  // |Renderer|
  bool IsValid() const override;

  // |Renderer|
  bool Setup() override;

  // |Renderer|
  bool Render(vk::CommandBuffer render_command_buffer) override;

  // |Renderer|
  bool Teardown() override;

  P_DISALLOW_COPY_AND_ASSIGN(ModelRenderer);
};

}  // namespace pixel