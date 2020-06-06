#pragma once

#include <map>
#include <memory>
#include <vector>

#include "macros.h"
#include "model.h"
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
