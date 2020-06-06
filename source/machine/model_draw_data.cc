#include "model_draw_data.h"

namespace pixel {
namespace model {

ModelDrawCall::ModelDrawCall(
    vk::PrimitiveTopology topology,
    std::vector<uint32_t> indices,
    std::vector<pixel::shaders::model_renderer::Vertex> vertices,
    ModelTextureMap textures)
    : topology_(topology),
      indices_(std::move(indices)),
      vertices_(std::move(vertices)),
      textures_(std::move(textures)) {}

ModelDrawCall::~ModelDrawCall() = default;

// *****************************************************************************
// *** ModelDrawCallBuilder
// *****************************************************************************

ModelDrawCallBuilder::ModelDrawCallBuilder() = default;

ModelDrawCallBuilder::~ModelDrawCallBuilder() = default;

ModelDrawCallBuilder& ModelDrawCallBuilder::SetTopology(
    vk::PrimitiveTopology topology) {
  topology_ = topology;
  return *this;
}

ModelDrawCallBuilder& ModelDrawCallBuilder::SetIndices(
    std::vector<uint32_t> indices) {
  indices_ = std::move(indices);
  return *this;
}

ModelDrawCallBuilder& ModelDrawCallBuilder::SetVertices(
    std::vector<pixel::shaders::model_renderer::Vertex> vertices) {
  vertices_ = std::move(vertices);
  return *this;
}

ModelDrawCallBuilder& ModelDrawCallBuilder::SetTexture(
    TextureType type,
    std::shared_ptr<Image> image,
    std::shared_ptr<Sampler> sampler) {
  textures_[type] = std::make_pair(std::move(image), std::move(sampler));
  return *this;
}

std::shared_ptr<ModelDrawCall> ModelDrawCallBuilder::CreateDrawCall() {
  return std::make_shared<ModelDrawCall>(topology_, std::move(indices_),
                                         std::move(vertices_),
                                         std::move(textures_));
}

// *****************************************************************************
// *** ModelDrawData
// *****************************************************************************

ModelDrawData::ModelDrawData() = default;

ModelDrawData::~ModelDrawData() = default;

void ModelDrawData::AddDrawCall(std::shared_ptr<ModelDrawCall> draw_call) {
  if (!draw_call) {
    return;
  }

  draw_calls_.emplace_back(std::move(draw_call));
}

}  // namespace model
}  // namespace pixel
