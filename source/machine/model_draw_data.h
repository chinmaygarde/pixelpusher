#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "macros.h"
#include "mapping.h"
#include "memory_allocator.h"
#include "model.h"
#include "rendering_context.h"
#include "shader_library.h"
// TODO: Move the contents of this file into this one.
#include "shaders/model_renderer.h"
#include "vulkan.h"

namespace pixel {
namespace model {

using ModelTextureMap =
    std::map<TextureType,
             std::pair<std::shared_ptr<Image>, std::shared_ptr<Sampler>>>;

class ModelDrawCall {
 public:
  using VertexValueType = pixel::shaders::model_renderer::Vertex;
  using IndexValueType = uint32_t;

  ModelDrawCall(vk::PrimitiveTopology topology,
                std::vector<uint32_t> indices,
                std::vector<pixel::shaders::model_renderer::Vertex> vertices,
                ModelTextureMap textures);

  ~ModelDrawCall();

  vk::PrimitiveTopology GetTopology() const;

  const std::vector<uint32_t>& GetIndices() const;

  const std::vector<pixel::shaders::model_renderer::Vertex>& GetVertices()
      const;

  const ModelTextureMap& GetTextures() const;

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

struct ModelDeviceDrawData {
  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleStrip;
  vk::DeviceSize vertex_buffer_offset = 0;
  vk::DeviceSize index_buffer_offset = 0;
  size_t index_count = 0;
  size_t vertex_count = 0;
};

class ModelDeviceContext {
 public:
  ModelDeviceContext(std::shared_ptr<RenderingContext> context,
                     std::unique_ptr<pixel::Buffer> vertex_buffer,
                     std::unique_ptr<pixel::Buffer> index_buffer,
                     std::vector<ModelDeviceDrawData> draw_data,
                     std::string debug_name);

  ~ModelDeviceContext();

  UniformBuffer<shaders::model_renderer::UniformBuffer>& GetUniformBuffer();

  bool Render(vk::CommandBuffer buffer);

  bool IsValid() const;

 private:
  std::shared_ptr<RenderingContext> context_;
  const std::string debug_name_;
  const std::vector<ModelDeviceDrawData> draw_data_;
  std::unique_ptr<ShaderLibrary> shader_library_;
  vk::UniqueDescriptorSetLayout descriptor_set_layout_;
  vk::UniquePipelineLayout pipeline_layout_;
  std::unique_ptr<pixel::Buffer> vertex_buffer_;
  std::unique_ptr<pixel::Buffer> index_buffer_;
  UniformBuffer<shaders::model_renderer::UniformBuffer> uniform_buffer_;
  DescriptorSets descriptor_sets_;
  std::set<vk::PrimitiveTopology> required_topologies_;
  std::map<vk::PrimitiveTopology, vk::UniquePipeline> pipelines_;
  bool is_valid_ = false;

  bool CreateShaderLibrary();

  bool CreateDescriptorSetLayout();

  bool CreateDescriptorSets();

  bool BindDescriptorSets();

  bool CreatePipelineLayout();

  bool CreateUniformBuffer();

  bool CreatePipelines();

  void OnShaderLibraryDidUpdate();

  P_DISALLOW_COPY_AND_ASSIGN(ModelDeviceContext);
};

class ModelDrawData {
 public:
  ModelDrawData(std::string debug_name);

  ~ModelDrawData();

  bool AddDrawCall(std::shared_ptr<ModelDrawCall> draw_call);

  std::unique_ptr<ModelDeviceContext> CreateModelDeviceContext(
      std::shared_ptr<RenderingContext> context) const;

  bool RegisterSampler(std::shared_ptr<Sampler> sampler);

  bool RegisterImage(std::shared_ptr<Image> image);

 private:
  std::string debug_name_;
  std::vector<std::shared_ptr<const ModelDrawCall>> draw_calls_;

  std::unique_ptr<pixel::Buffer> CreateVertexBuffer(
      const RenderingContext& context) const;

  std::unique_ptr<pixel::Buffer> CreateIndexBuffer(
      const RenderingContext& context) const;

  P_DISALLOW_COPY_AND_ASSIGN(ModelDrawData);
};

}  // namespace model
}  // namespace pixel
