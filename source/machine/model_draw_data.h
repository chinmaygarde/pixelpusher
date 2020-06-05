#pragma once

#include <memory>
#include <vector>

#include "macros.h"
#include "shaders/model_renderer.h"

namespace pixel {
namespace model {

class ModelDrawCall {
 public:
 private:
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

  std::shared_ptr<ModelDrawCall> CreateDrawCall();

 private:
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
