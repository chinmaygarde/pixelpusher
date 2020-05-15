#include <gtest/gtest.h>

#include <future>

#include "asset_loader.h"
#include "assets_location.h"
#include "file.h"

namespace pixel {
namespace test {

TEST(AssetLoaderTest, CanCreateAndDestroyLoader) {
  AssetLoader loader;
};

TEST(AssetLoaderTest, CanLoadAsset) {
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
}

}  // namespace test
}  // namespace pixel
