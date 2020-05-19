#pragma once

#include <memory>

#include "macros.h"
#include "mapping.h"
#include "thread.h"
#include "tiny_gltf.h"

namespace pixel {

struct Asset {
  tinygltf::Model model;

  Asset(tinygltf::Model model) : model(std::move(model)) {}
};

class AssetLoader {
 public:
  static std::shared_ptr<AssetLoader> GetGlobal();

  AssetLoader();

  ~AssetLoader();

  void LoadAsset(std::string assets_base_dir,
                 std::string asset_file,
                 std::function<void(std::unique_ptr<Asset>)>);

 private:
  Thread thread_;

  P_DISALLOW_COPY_AND_ASSIGN(AssetLoader);
};

}  // namespace pixel
