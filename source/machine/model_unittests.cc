#include <gtest/gtest.h>

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
};

}  // namespace test
}  // namespace model
}  // namespace pixel
