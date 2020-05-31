#include <gtest/gtest.h>

#include <filesystem>
#include <future>

#include "asset_loader.h"
#include "assets_location.h"
#include "model.h"

namespace pixel {
namespace model {
namespace test {

TEST(ModelTest, CanLoadModelFromAsset) {
  auto base_dir = PIXEL_GLTF_MODELS_LOCATION "/DamagedHelmet/glTF";
  auto asset_file = "DamagedHelmet.gltf";

  AssetLoader loader;

  std::promise<std::unique_ptr<Asset>> asset_promise;
  auto future = asset_promise.get_future();

  loader.LoadAsset(
      base_dir, asset_file,
      MakeCopyable([promise = std::move(asset_promise)](auto asset) mutable {
        ASSERT_TRUE(asset);
        promise.set_value(std::move(asset));
      }));

  auto asset = future.get();
  ASSERT_TRUE(asset);

  Model model(*asset);

  ASSERT_EQ(model.GetScenes().size(), 1u);
  ASSERT_EQ(model.GetScenes()[0]->GetNodes().size(), 1u);
};

static std::optional<DrawData> GetDrawDataForModelName(
    const std::string& model_name) {
  AssetLoader loader;

  std::promise<std::unique_ptr<Asset>> asset_promise;
  auto future = asset_promise.get_future();

  std::filesystem::path base_dir;
  std::filesystem::path asset_file;

  base_dir /= PIXEL_GLTF_MODELS_LOCATION;
  base_dir /= model_name;
  base_dir /= "glTF";

  asset_file = model_name + ".gltf";

  base_dir.make_preferred();
  asset_file.make_preferred();

  loader.LoadAsset(
      base_dir.u8string(), asset_file.u8string(),
      MakeCopyable([promise = std::move(asset_promise)](auto asset) mutable {
        ASSERT_TRUE(asset);
        promise.set_value(std::move(asset));
      }));

  auto asset = future.get();

  if (!asset) {
    return std::nullopt;
  }

  auto model = model::Model{asset->model};
  return model.GetDrawData();
}

TEST(ModelTest, DrawDataTestDamagedHelmet) {
  auto draw_data_optional = GetDrawDataForModelName("DamagedHelmet");
  ASSERT_TRUE(draw_data_optional.has_value());
  const auto& draw_data = draw_data_optional.value();
  ASSERT_EQ(draw_data.ops.size(), 1u);
  ASSERT_EQ(draw_data.ops.count(vk::PrimitiveTopology::eTriangleFan), 1u);
  ASSERT_EQ(draw_data.ops.at(vk::PrimitiveTopology::eTriangleFan).size(), 1u);
  const auto& ops = draw_data.ops.at(vk::PrimitiveTopology::eTriangleFan);
  const auto& draw_op = ops[0];
  EXPECT_EQ(draw_op.vertices.size(), 14556u);
  EXPECT_EQ(draw_op.indices.size(), 46356u);
}

TEST(ModelTest, DrawDataTestTriangleWithoutIndices) {
  auto draw_data_optional = GetDrawDataForModelName("TriangleWithoutIndices");
  ASSERT_TRUE(draw_data_optional.has_value());
  const auto& draw_data = draw_data_optional.value();
  ASSERT_EQ(draw_data.ops.size(), 1u);
  ASSERT_EQ(draw_data.ops.count(vk::PrimitiveTopology::eTriangleFan), 1u);
  ASSERT_EQ(draw_data.ops.at(vk::PrimitiveTopology::eTriangleFan).size(), 1u);
  const auto& ops = draw_data.ops.at(vk::PrimitiveTopology::eTriangleFan);
  const auto& draw_op = ops[0];
  EXPECT_EQ(draw_op.vertices.size(), 3u);
  EXPECT_EQ(draw_op.indices.size(), 0u);
}

}  // namespace test
}  // namespace model
}  // namespace pixel
