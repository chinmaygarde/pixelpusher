#pragma once

#include <map>

#include "asset_loader.h"
#include "macros.h"
#include "model.h"
#include "renderer.h"
#include "shader_library.h"
#include "shaders/model_renderer.h"
#include "unshared_weak.h"
#include "vulkan.h"

namespace pixel {

class ModelRenderer : public Renderer {
 public:
  ModelRenderer(std::shared_ptr<RenderingContext> context,
                std::string model_assets_dir,
                std::string model_path,
                std::string debug_name);

  // |Renderer|
  ~ModelRenderer() override;

 private:
  enum class DrawType {
    kIndexed,
    kVertex,

  };
  struct DrawData {
    vk::PrimitiveTopology topology;
    vk::DeviceSize vertex_buffer_offset = 0;
    vk::DeviceSize index_buffer_offset = 0;
    size_t index_count = 0;
    size_t vertex_count = 0;
    DrawType type = DrawType::kIndexed;
  };

  std::unique_ptr<const model::Model> model_;
  std::vector<DrawData> draw_data_;
  std::vector<vk::PrimitiveTopology> required_topologies_;
  bool is_valid_ = false;

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

  void OnShaderLibraryDidUpdate();

  bool RebuildPipelines();

  P_DISALLOW_COPY_AND_ASSIGN(ModelRenderer);
};

}  // namespace pixel
