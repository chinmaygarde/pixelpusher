#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "asset_loader.h"
#include "glm.h"
#include "macros.h"
#include "vulkan.h"

namespace pixel {
namespace model {

class Model;
class Accessor;
class Animation;
class Buffer;
class BufferView;
class Material;
class Primitive;
class Mesh;
class Node;
class Texture;
class Image;
class Skin;
class Sampler;
class Camera;
class Scene;
class Light;

template <class T>
using Collection = std::vector<std::shared_ptr<T>>;

using Accessors = Collection<Accessor>;
using Animations = Collection<Animation>;
using Buffers = Collection<Buffer>;
using BufferViews = Collection<BufferView>;
using Materials = Collection<Material>;
using Meshes = Collection<Mesh>;
using Nodes = Collection<Node>;
using Textures = Collection<Texture>;
using Images = Collection<Image>;
using Skins = Collection<Skin>;
using Samplers = Collection<Sampler>;
using Cameras = Collection<Camera>;
using Scenes = Collection<Scene>;
using Lights = Collection<Light>;

template <class GLTFType>
class GLTFArchivable {
 public:
  virtual void ReadFromArchive(const GLTFType& archive_member) = 0;

  virtual void ResolveReferences(const Model& model,
                                 const GLTFType& archive_member) = 0;
};

class Accessor final : public GLTFArchivable<tinygltf::Accessor> {
 public:
  Accessor();

  ~Accessor();

  void ReadFromArchive(const tinygltf::Accessor& accessor) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Accessor& accessor) override;

 private:
  std::string name_;
  std::shared_ptr<BufferView> buffer_view_;

  size_t byte_offset_ = 0;
  bool normalized_ = false;
  size_t stride_ = 0;

  size_t count_ = 0;
  std::vector<double> min_values_;
  std::vector<double> max_values_;
  // TODO: Sparse.

  P_DISALLOW_COPY_AND_ASSIGN(Accessor);
};

class Animation final : public GLTFArchivable<tinygltf::Animation> {
 public:
  Animation();

  ~Animation();

  void ReadFromArchive(const tinygltf::Animation& animation) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Animation& animation) override;

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Animation);
};

class Buffer final : public GLTFArchivable<tinygltf::Buffer> {
 public:
  Buffer();

  ~Buffer();

  void ReadFromArchive(const tinygltf::Buffer& buffer) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Buffer& buffer) override;

 private:
  std::string name_;
  std::unique_ptr<Mapping> data_;
  std::string uri_;

  P_DISALLOW_COPY_AND_ASSIGN(Buffer);
};

class BufferView final : public GLTFArchivable<tinygltf::BufferView> {
 public:
  BufferView();

  ~BufferView();

  void ReadFromArchive(const tinygltf::BufferView& view) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::BufferView& view) override;

 private:
  std::string name_;
  std::shared_ptr<Buffer> buffer_;
  size_t byte_offset_ = 0;
  size_t byte_length_ = 0;
  size_t byte_stride_ = 0;
  // TODO: Target. Don't know what that means.

  P_DISALLOW_COPY_AND_ASSIGN(BufferView);
};

class Material final : public GLTFArchivable<tinygltf::Material> {
 public:
  Material();

  ~Material();

  void ReadFromArchive(const tinygltf::Material& material) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Material& material) override;

 private:
  // TOOD

  P_DISALLOW_COPY_AND_ASSIGN(Material);
};

class Primitive final : public GLTFArchivable<tinygltf::Primitive> {
 public:
  Primitive();

  ~Primitive();

  void ReadFromArchive(const tinygltf::Primitive& primitive) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Primitive& primitive) override;

 private:
  std::map<std::string, std::shared_ptr<Accessor>> attributes_;
  std::shared_ptr<Material> material_;
  std::shared_ptr<Accessor> indices_;
  vk::PrimitiveTopology mode_;
  // TODO: Target for morph targets.

  P_DISALLOW_COPY_AND_ASSIGN(Primitive);
};

class Mesh final : public GLTFArchivable<tinygltf::Mesh> {
 public:
  Mesh();

  ~Mesh();

  void ReadFromArchive(const tinygltf::Mesh& mesh) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Mesh& mesh) override;

 private:
  std::string name_;
  std::vector<std::shared_ptr<Primitive>> primitives_;
  std::vector<double> weights_;

  P_DISALLOW_COPY_AND_ASSIGN(Mesh);
};

class Node final : public GLTFArchivable<tinygltf::Node> {
 public:
  Node();

  ~Node();

  void ReadFromArchive(const tinygltf::Node& node) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Node& node) override;

 private:
  std::string name_;
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<Skin> skin_;
  std::shared_ptr<Mesh> mesh_;
  std::vector<std::shared_ptr<Node>> children_;
  glm::vec4 rotation_ = glm::vec4(0.0);
  glm::vec3 scale_ = glm::vec4(1.0);
  glm::vec3 translation_ = glm::vec3(0.0);
  glm::mat4 matrix_ = glm::mat4();
  std::vector<double> weights_;

  P_DISALLOW_COPY_AND_ASSIGN(Node);
};

class Texture final : public GLTFArchivable<tinygltf::Texture> {
 public:
  Texture();

  ~Texture();

  void ReadFromArchive(const tinygltf::Texture& texture) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Texture& texture) override;

 private:
  std::string name_;
  std::shared_ptr<Sampler> sampler_;
  std::shared_ptr<Image> source_;

  P_DISALLOW_COPY_AND_ASSIGN(Texture);
};

class Image final : public GLTFArchivable<tinygltf::Image> {
 public:
  Image();

  ~Image();

  void ReadFromArchive(const tinygltf::Image& image) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Image& image) override;

 private:
  std::string name_;
  size_t width_ = 0;
  size_t height_ = 0;
  size_t components_ = 0;
  size_t bits_per_channel_ = 0;
  size_t pixel_stride_ = 0;
  // Default image decompression can be disabled by the Image::as_is flag and a
  // custom image decompression routine.
  std::unique_ptr<Mapping> decompressed_image_;
  std::shared_ptr<BufferView> buffer_view_;
  std::string mime_type_;
  std::string uri_;

  P_DISALLOW_COPY_AND_ASSIGN(Image);
};

class Skin final : public GLTFArchivable<tinygltf::Skin> {
 public:
  Skin();

  ~Skin();

  void ReadFromArchive(const tinygltf::Skin& skin) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Skin& skin) override;

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Skin);
};

class Sampler final : public GLTFArchivable<tinygltf::Sampler> {
 public:
  Sampler();

  ~Sampler();

  void ReadFromArchive(const tinygltf::Sampler& sampler) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Sampler& sampler) override;

 private:
  std::string name_;
  vk::Filter min_filter_;
  vk::Filter mag_filter_;
  vk::SamplerMipmapMode mipmap_mode_;
  vk::SamplerAddressMode wrap_s_;
  vk::SamplerAddressMode wrap_t_;
  vk::SamplerAddressMode wrap_r_;

  P_DISALLOW_COPY_AND_ASSIGN(Sampler);
};

class Camera final : public GLTFArchivable<tinygltf::Camera> {
 public:
  Camera();

  ~Camera();

  void ReadFromArchive(const tinygltf::Camera& camera) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Camera& camera) override;

 private:
  std::string name_;
  struct PerspectiveCamera {
    double aspect_ratio = 0.0;
    double y_fov = 0.0;
    double z_far = 0.0;
    double z_near = 0.0;
  };

  struct OrthographicCamera {
    double x_mag = 0.0;
    double y_mag = 0.0;
    double z_far = 0.0;
    double z_near = 0.0;
  };

  std::variant<PerspectiveCamera, OrthographicCamera> projection_ =
      OrthographicCamera{};

  P_DISALLOW_COPY_AND_ASSIGN(Camera);
};

class Scene final : public GLTFArchivable<tinygltf::Scene> {
 public:
  Scene();

  ~Scene();

  void ReadFromArchive(const tinygltf::Scene& scene) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Scene& scene) override;

 private:
  std::string name_;
  std::vector<std::shared_ptr<Node>> nodes_;

  P_DISALLOW_COPY_AND_ASSIGN(Scene);
};

class Light final : public GLTFArchivable<tinygltf::Light> {
 public:
  Light();

  ~Light();

  void ReadFromArchive(const tinygltf::Light& light) override;

  void ResolveReferences(const Model& model,
                         const tinygltf::Light& light) override;

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Light);
};

class Model {
 public:
  Model(const Asset& asset);

  ~Model();

  const Accessors& GetAccessors() const;

  const Animations& GetAnimations() const;

  const Buffers& GetBuffers() const;

  const BufferViews& GetBufferViews() const;

  const Materials& GetMaterials() const;

  const Meshes& GetMeshes() const;

  const Nodes& GetNodes() const;

  const Textures& GetTextures() const;

  const Images& GetImages() const;

  const Skins& GetSkins() const;

  const Samplers& GetSamplers() const;

  const Cameras& GetCameras() const;

  const Scenes& GetScenes() const;

  const Lights& GetLights() const;

 private:
  friend class Accessor;
  friend class Animation;
  friend class Buffer;
  friend class BufferView;
  friend class Material;
  friend class Primitive;
  friend class Mesh;
  friend class Node;
  friend class Texture;
  friend class Image;
  friend class Skin;
  friend class Sampler;
  friend class Camera;
  friend class Scene;
  friend class Light;

  Accessors accessors_;
  Animations animations_;
  Buffers buffers_;
  BufferViews bufferViews_;
  Materials materials_;
  Meshes meshes_;
  Nodes nodes_;
  Textures textures_;
  Images images_;
  Skins skins_;
  Samplers samplers_;
  Cameras cameras_;
  Scenes scenes_;
  Lights lights_;

  P_DISALLOW_COPY_AND_ASSIGN(Model);
};

}  // namespace model
}  // namespace pixel
