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
    stride_ = std::max<int>(
        0u, tinygltf::GetComponentSizeInBytes(accessor.componentType));
    count_ = accessor.count;
    min_values_ = accessor.minValues;
    max_values_ = accessor.maxValues;
  }

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
  void ReadFromArchive(const tinygltf::Animation& animation) {}

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Animation);
};

class Buffer final : public GLTFArchivable<tinygltf::Buffer> {
 public:
  void ReadFromArchive(const tinygltf::Buffer& buffer) {
    name_ = buffer.name;
    data_ = CopyMapping(buffer.data.data(), buffer.data.size());
    uri_ = buffer.uri;
  }

 private:
  std::string name_;
  std::unique_ptr<Mapping> data_;
  std::string uri_;

  P_DISALLOW_COPY_AND_ASSIGN(Buffer);
};

class BufferView final : public GLTFArchivable<tinygltf::BufferView> {
 public:
  void ReadFromArchive(const tinygltf::BufferView& view) {
    name_ = view.name;
    byte_offset_ = view.byteOffset;
    byte_length_ = view.byteLength;
    byte_stride_ = view.byteStride;
  }

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
  void ReadFromArchive(const tinygltf::Material& material) {}

 private:
  // TOOD

  P_DISALLOW_COPY_AND_ASSIGN(Material);
};

class Primitive final : public GLTFArchivable<tinygltf::Primitive> {
 public:
  void ReadFromArchive(const tinygltf::Primitive& primitive) {
    switch (primitive.mode) {
      case TINYGLTF_MODE_POINTS:
        mode_ = vk::PrimitiveTopology::ePointList;
      case TINYGLTF_MODE_LINE:
        mode_ = vk::PrimitiveTopology::eLineList;
      case TINYGLTF_MODE_LINE_LOOP:
        // TODO: WTF is this?
        // mode_ = vk::PrimitiveTopology::elineloop;
      case TINYGLTF_MODE_LINE_STRIP:
        mode_ = vk::PrimitiveTopology::eLineStrip;
      case TINYGLTF_MODE_TRIANGLES:
        mode_ = vk::PrimitiveTopology::eTriangleList;
      case TINYGLTF_MODE_TRIANGLE_STRIP:
        mode_ = vk::PrimitiveTopology::eTriangleStrip;
      case TINYGLTF_MODE_TRIANGLE_FAN:
        mode_ = vk::PrimitiveTopology::eTriangleFan;
    }
  }

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
  std::string name_;
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
  void ReadFromArchive(const tinygltf::Texture& texture) {
    name_ = texture.name;
  }

 private:
  std::string name_;
  std::shared_ptr<Sampler> sampler_;
  std::shared_ptr<Image> source_;

  P_DISALLOW_COPY_AND_ASSIGN(Texture);
};

class Image final : public GLTFArchivable<tinygltf::Image> {
 public:
  void ReadFromArchive(const tinygltf::Image& image) {
    name_ = image.name;
    width_ = std::max<int>(0u, image.width);
    height_ = std::max<int>(0u, image.height);
    components_ = std::max<int>(0u, image.component);
    bits_per_channel_ = std::max<int>(0u, image.bits);
    pixel_stride_ =
        std::max<int>(0u, tinygltf::GetComponentSizeInBytes(image.pixel_type));
    decompressed_image_ = CopyMapping(image.image.data(), image.image.size());
    mime_type_ = image.mimeType;
    uri_ = image.uri;
  }

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
  void ReadFromArchive(const tinygltf::Skin& skin) {}

 private:
  // TODO
  P_DISALLOW_COPY_AND_ASSIGN(Skin);
};

inline std::pair<vk::Filter, vk::SamplerMipmapMode> ParseVkFilter(int filter) {
  switch (filter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
      return {vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest};
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
      return {vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest};
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
      return {vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest};
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
      return {vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest};
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
      return {vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear};
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      return {vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear};
  }
  return {{}, {}};
}

inline vk::SamplerAddressMode ParseVkSamplerAddressMode(int mode) {
  switch (mode) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      return vk::SamplerAddressMode::eRepeat;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      return vk::SamplerAddressMode::eClampToEdge;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      return vk::SamplerAddressMode::eMirroredRepeat;
  }
  return {};
}

class Sampler final : public GLTFArchivable<tinygltf::Sampler> {
 public:
  void ReadFromArchive(const tinygltf::Sampler& sampler) {
    name_ = sampler.name;
    // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/schema/sampler.schema.json
    std::tie(mag_filter_, std::ignore) = ParseVkFilter(sampler.magFilter);
    std::tie(min_filter_, mipmap_mode_) = ParseVkFilter(sampler.minFilter);
    wrap_s_ = ParseVkSamplerAddressMode(sampler.wrapS);
    wrap_t_ = ParseVkSamplerAddressMode(sampler.wrapT);
    wrap_r_ = ParseVkSamplerAddressMode(sampler.wrapR);
  }

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
