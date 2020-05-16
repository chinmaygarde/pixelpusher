#include "model.h"

namespace pixel {

template <class ResourceType, class GLTFResourceType>
std::vector<std::shared_ptr<ResourceType>> Inflate(
    const std::vector<GLTFResourceType>& items) {
  auto collection = std::vector<std::shared_ptr<ResourceType>>{};
  collection.reserve(items.size());
  for (const auto& item : items) {
    auto resource = std::make_shared<ResourceType>();
    resource->ReadFromArchive(item);
    collection.emplace_back(std::move(resource));
  }
  return collection;
}

Model::Model(const Asset& asset) {
  accessors_ = Inflate<Accessor, tinygltf::Accessor>(asset.model.accessors);
  animations_ = Inflate<Animation, tinygltf::Animation>(asset.model.animations);
  buffers_ = Inflate<Buffer, tinygltf::Buffer>(asset.model.buffers);
  bufferViews_ =
      Inflate<BufferView, tinygltf::BufferView>(asset.model.bufferViews);
  materials_ = Inflate<Material, tinygltf::Material>(asset.model.materials);
  meshes_ = Inflate<Mesh, tinygltf::Mesh>(asset.model.meshes);
  nodes_ = Inflate<Node, tinygltf::Node>(asset.model.nodes);
  textures_ = Inflate<Texture, tinygltf::Texture>(asset.model.textures);
  images_ = Inflate<Image, tinygltf::Image>(asset.model.images);
  skins_ = Inflate<Skin, tinygltf::Skin>(asset.model.skins);
  samplers_ = Inflate<Sampler, tinygltf::Sampler>(asset.model.samplers);
  cameras_ = Inflate<Camera, tinygltf::Camera>(asset.model.cameras);
  scenes_ = Inflate<Scene, tinygltf::Scene>(asset.model.scenes);
  lights_ = Inflate<Light, tinygltf::Light>(asset.model.lights);
}

Model::~Model() = default;

void Mesh::ReadFromArchive(const tinygltf::Mesh& mesh) {
  name_ = mesh.name;
  primitives_ = Inflate<Primitive, tinygltf::Primitive>(mesh.primitives);
  weights_ = mesh.weights;
}

}  // namespace pixel
