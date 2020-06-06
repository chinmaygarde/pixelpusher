#pragma once

#include <map>
#include <memory>
#include <vector>

#include "macros.h"
#include "memory_allocator.h"
#include "model.h"
#include "rendering_context.h"
#include "shader_library.h"
#include "shaders/model_renderer.h"
#include "vulkan.h"

namespace pixel {
namespace model {

using ModelTextureMap =
    std::map<TextureType,
             std::pair<std::shared_ptr<Image>, std::shared_ptr<Sampler>>>;

class ModelDrawCall {
 public:
  ModelDrawCall(vk::PrimitiveTopology topology,
                std::vector<uint32_t> indices,
                std::vector<pixel::shaders::model_renderer::Vertex> vertices,
                ModelTextureMap textures);

  ~ModelDrawCall();

 private:
  vk::PrimitiveTopology topology_;
  std::vector<uint32_t> indices_;
  std::vector<pixel::shaders::model_renderer::Vertex> vertices_;
  ModelTextureMap textures_;

  P_DISALLOW_COPY_AND_ASSIGN(ModelDrawCall);
};

class ModelDrawCallBuilder {
 public:
  ModelDrawCallBuilder();

  ~ModelDrawCallBuilder();

  ModelDrawCallBuilder& SetTopology(vk::PrimitiveTopology topology);

  ModelDrawCallBuilder& SetIndices(std::vector<uint32_t> indices);

  ModelDrawCallBuilder& SetVertices(
      std::vector<pixel::shaders::model_renderer::Vertex> vertices);

  ModelDrawCallBuilder& SetTexture(TextureType type,
                                   std::shared_ptr<Image> image,
                                   std::shared_ptr<Sampler> sampler);

  std::shared_ptr<ModelDrawCall> CreateDrawCall();

 private:
  vk::PrimitiveTopology topology_ = vk::PrimitiveTopology::eTriangleStrip;
  std::vector<uint32_t> indices_;
  std::vector<pixel::shaders::model_renderer::Vertex> vertices_;
  ModelTextureMap textures_;

  P_DISALLOW_COPY_AND_ASSIGN(ModelDrawCallBuilder);
};

class ModelDeviceContext {
 public:
  ModelDeviceContext(std::shared_ptr<RenderingContext> context,
                     std::string debug_name);

  ~ModelDeviceContext();

 private:
  std::shared_ptr<RenderingContext> context_;
  const std::string debug_name_;
  std::unique_ptr<ShaderLibrary> shader_library_;
  vk::UniqueDescriptorSetLayout descriptor_set_layout_;
  vk::UniquePipelineLayout pipeline_layout_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> index_buffer_;
  UniformBuffer<shaders::model_renderer::UniformBuffer> uniform_buffer_;
  DescriptorSets descriptor_sets_;
  std::vector<vk::PrimitiveTopology> required_topologies_;
  std::map<vk::PrimitiveTopology, vk::UniquePipeline> pipelines_;
  bool is_valid_ = false;

  bool CreateShaderLibrary();

  bool CreateDescriptorSetLayout();

  bool CreateDescriptorSets();

  bool BindDescriptorSets();

  bool CreatePipelineLayout();

  bool CreateVertexBuffer(
      const std::vector<pixel::shaders::model_renderer::Vertex>& vertices);

  bool CreateIndexBuffer(const std::vector<uint32_t>& indices);

  bool CreateUniformBuffer();

  bool CreatePipelines();

  void OnShaderLibraryDidUpdate();

  P_DISALLOW_COPY_AND_ASSIGN(ModelDeviceContext);
};

class ModelDrawData {
 public:
  ModelDrawData();

  ~ModelDrawData();

  void AddDrawCall(std::shared_ptr<ModelDrawCall> draw_call);

 private:
  std::vector<std::shared_ptr<ModelDrawCall>> draw_calls_;

  P_DISALLOW_COPY_AND_ASSIGN(ModelDrawData);
};

}  // namespace model
}  // namespace pixel
