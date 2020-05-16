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

inline void ArchiveRead(glm::vec4& ret, const std::vector<double>& input) {
  if (input.size() != 4) {
    return;
  }
  ret = {input[0], input[1], input[2], input[3]};
}

inline void ArchiveRead(glm::vec3& ret, const std::vector<double>& input) {
  if (input.size() != 3) {
    return;
  }

  ret = {input[0], input[1], input[2]};
}

inline void ArchiveRead(glm::mat4& ret, const std::vector<double>& input) {
  if (input.size() != 16) {
    return;
  }

  ret = {
      //
      input[0], input[4], input[8],  input[12],  //
      input[1], input[5], input[9],  input[13],  //
      input[2], input[6], input[10], input[14],  //
      input[3], input[7], input[11], input[15],  //
  };
}

template <class GLTFType>
class GLTFArchivable {
 public:
  void ReadFromArchive(const GLTFType& archive_member);
};

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

class Accessor final : public GLTFArchivable<tinygltf::Accessor> {
 public:
  void ReadFromArchive(const tinygltf::Accessor& accessor) {
    name_ = accessor.name;
    byte_offset_ = accessor.byteOffset;
    normalized_ = accessor.normalized;
  }

 private:
  std::string name_;
  std::shared_ptr<BufferView> buffer_view_;

  size_t byte_offset_ = 0;
  bool normalized_ = false;
  vk::Format component_type_;
  size_t count_ = 0;
  std::vector<double> min_values_;
  std::vector<double> max_values_;
  // TODO: Sparse.

  P_DISALLOW_COPY_AND_ASSIGN(Accessor);
};

class Animation final : public GLTFArchivable<tinygltf::Animation> {
 public:
  void ReadFromArchive(const tinygltf::Animation& animation) {}

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Animation);
};

class Buffer final : public GLTFArchivable<tinygltf::Buffer> {
 public:
  void ReadFromArchive(const tinygltf::Buffer& buffer) {}

 private:
  std::unique_ptr<Mapping> data_;
  std::string uri_;

  P_DISALLOW_COPY_AND_ASSIGN(Buffer);
};

class BufferView final : public GLTFArchivable<tinygltf::BufferView> {
 public:
  void ReadFromArchive(const tinygltf::BufferView& view) {}

 private:
  std::shared_ptr<Buffer> buffer_;
  size_t byte_offset_ = 0;
  size_t byte_length_ = 0;
  size_t byte_stride_ = 0;
  // TODO: Target. Don't know what that means.

  P_DISALLOW_COPY_AND_ASSIGN(BufferView);
};

class Material final : public GLTFArchivable<tinygltf::Material> {
 public:
  void ReadFromArchive(const tinygltf::Material& material) {}

 private:
  // TOOD

  P_DISALLOW_COPY_AND_ASSIGN(Material);
};

class Primitive final : public GLTFArchivable<tinygltf::Primitive> {
 public:
  void ReadFromArchive(const tinygltf::Primitive& primitive) {}

 private:
  std::map<std::string, std::shared_ptr<Accessor>> attributes_;
  std::shared_ptr<Material> material_;
  std::shared_ptr<Accessor> indices_;
  vk::PrimitiveTopology mode_;
  // TODO: Target for morph targets.
};

class Mesh final : public GLTFArchivable<tinygltf::Mesh> {
 public:
  void ReadFromArchive(const tinygltf::Mesh& mesh);

 private:
  std::vector<std::shared_ptr<Primitive>> primitives_;
  std::vector<double> weights_;

  P_DISALLOW_COPY_AND_ASSIGN(Mesh);
};

class Node final : public GLTFArchivable<tinygltf::Node> {
 public:
  void ReadFromArchive(const tinygltf::Node& node) {
    name_ = node.name;
    ArchiveRead(rotation_, node.rotation);
    ArchiveRead(scale_, node.rotation);
    ArchiveRead(translation_, node.rotation);
    ArchiveRead(matrix_, node.rotation);
    weights_ = node.weights;
  }

 private:
  glm::vec4 rotation_ = glm::vec4(0.0);
  glm::vec3 scale_ = glm::vec4(1.0);
  glm::vec3 translation_ = glm::vec3(0.0);
  glm::mat4 matrix_ = glm::mat4();
  std::vector<double> weights_;

  P_DISALLOW_COPY_AND_ASSIGN(Node);
};

class Texture final : public GLTFArchivable<tinygltf::Texture> {
 public:
  void ReadFromArchive(const tinygltf::Texture& texture) {}

 private:
  std::shared_ptr<Sampler> sample_;
  std::shared_ptr<Image> source_;

  P_DISALLOW_COPY_AND_ASSIGN(Texture);
};

class Image final : public GLTFArchivable<tinygltf::Image> {
 public:
  void ReadFromArchive(const tinygltf::Image& image) {}

 private:
  size_t width_ = 0;
  size_t height_ = 0;
  size_t component_ = 0;
  size_t bits_per_channel_ = 0;
  vk::Format pixel_format_;
  std::shared_ptr<BufferView> buffer_view_;
  std::string mime_type_;
  std::string uri_;

  P_DISALLOW_COPY_AND_ASSIGN(Image);
};

class Skin final : public GLTFArchivable<tinygltf::Skin> {
 public:
  void ReadFromArchive(const tinygltf::Skin& skin) {}

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Skin);
};

class Sampler final : public GLTFArchivable<tinygltf::Sampler> {
 public:
  void ReadFromArchive(const tinygltf::Sampler& sampler) {}

 private:
  vk::Filter min_filter_;
  vk::Filter mag_filter_;
  vk::SamplerAddressMode wrap_s_;
  vk::SamplerAddressMode wrap_t_;
  vk::SamplerAddressMode wrap_r_;

  P_DISALLOW_COPY_AND_ASSIGN(Sampler);
};

class Camera final : public GLTFArchivable<tinygltf::Camera> {
 public:
  void ReadFromArchive(const tinygltf::Camera& camera) {}

 private:
  struct PerspectiveCamera {
    double aspect_ratio_ = 0.0;
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

  std::variant<PerspectiveCamera, OrthographicCamera> projection_;

  P_DISALLOW_COPY_AND_ASSIGN(Camera);
};

class Scene final : public GLTFArchivable<tinygltf::Scene> {
 public:
  void ReadFromArchive(const tinygltf::Scene& scene) {}

 private:
  std::vector<std::shared_ptr<Node>> nodes_;

  P_DISALLOW_COPY_AND_ASSIGN(Scene);
};

class Light final : public GLTFArchivable<tinygltf::Light> {
 public:
  void ReadFromArchive(const tinygltf::Light& light) {}

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Light);
};

class Model {
 public:
  Model(const Asset& asset);

  ~Model();

 private:
  std::vector<std::shared_ptr<Accessor>> accessors_;
  std::vector<std::shared_ptr<Animation>> animations_;
  std::vector<std::shared_ptr<Buffer>> buffers_;
  std::vector<std::shared_ptr<BufferView>> bufferViews_;
  std::vector<std::shared_ptr<Material>> materials_;
  std::vector<std::shared_ptr<Mesh>> meshes_;
  std::vector<std::shared_ptr<Node>> nodes_;
  std::vector<std::shared_ptr<Texture>> textures_;
  std::vector<std::shared_ptr<Image>> images_;
  std::vector<std::shared_ptr<Skin>> skins_;
  std::vector<std::shared_ptr<Sampler>> samplers_;
  std::vector<std::shared_ptr<Camera>> cameras_;
  std::vector<std::shared_ptr<Scene>> scenes_;
  std::vector<std::shared_ptr<Light>> lights_;

  P_DISALLOW_COPY_AND_ASSIGN(Model);
};

}  // namespace pixel
