#include "asset_loader.h"

#include <sstream>

#include "file.h"
#include "logging.h"

namespace pixel {

std::shared_ptr<AssetLoader> AssetLoader::GetGlobal() {
  static auto sLoader = std::make_shared<AssetLoader>();
  return sLoader;
}

AssetLoader::AssetLoader() : thread_("AssetLoader") {}

AssetLoader::~AssetLoader() {
  thread_.Terminate();
}

static std::unique_ptr<Asset> LoadAssetFromMapping(
    const Mapping& mapping,
    std::string assets_base_dir) {
  tinygltf::TinyGLTF loader;

  tinygltf::Model model;
  std::string errors, warnings;

  if (!loader.LoadASCIIFromString(
          &model, &errors, &warnings,
          reinterpret_cast<const char*>(mapping.GetData()), mapping.GetSize(),
          assets_base_dir)) {
    P_ERROR << "Could not load GLTF file: " << errors;
  }

  if (warnings.size() != 0) {
    P_ERROR << "Warnings while loading model: " << warnings;
  }

  return std::make_unique<Asset>(std::move(model));
}

static std::unique_ptr<Mapping> GetAssetFileMapping(
    const std::string& assets_base_dir,
    const std::string& asset_file) {
  std::stringstream stream;
  stream << assets_base_dir << "/" << asset_file;
  auto file_path = stream.str();

  auto mapping = OpenFile(file_path.c_str());
  if (!mapping) {
    P_ERROR << "Could not open file: " << file_path;
    return nullptr;
  }

  return mapping;
}

void AssetLoader::LoadAsset(
    std::string assets_base_dir,
    std::string asset_file,
    std::function<void(std::unique_ptr<Asset>)> on_done) {
  if (!on_done) {
    return;
  }

  thread_.GetDispatcher()->PostTask(
      MakeCopyable([assets_base_dir = std::move(assets_base_dir),
                    asset_file = std::move(asset_file), on_done] {
        auto mapping = GetAssetFileMapping(assets_base_dir, asset_file);
        if (!mapping) {
          on_done(nullptr);
          return;
        }
        on_done(LoadAssetFromMapping(*mapping, std::move(assets_base_dir)));
      }));
}

}  // namespace pixel
