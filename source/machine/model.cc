#include "model.h"

#include <type_traits>

namespace pixel {
namespace model {

template <class ResourceType, class GLTFResourceType>
static std::vector<std::shared_ptr<ResourceType>> Inflate(
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

template <class ResourceTypePtr, class GLTFResourceType>
static void ResolveCollectionReferences(
    const Model& model,
    std::vector<ResourceTypePtr>& collection,
    const std::vector<GLTFResourceType>& items) {
  P_ASSERT(collection.size() == items.size());

  for (size_t i = 0, count = collection.size(); i < count; i++) {
    collection[i]->ResolveReferences(model, items[i]);
  }
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

  ResolveCollectionReferences(*this, accessors_, asset.model.accessors);
  ResolveCollectionReferences(*this, animations_, asset.model.animations);
  ResolveCollectionReferences(*this, buffers_, asset.model.buffers);
  ResolveCollectionReferences(*this, bufferViews_, asset.model.bufferViews);
  ResolveCollectionReferences(*this, materials_, asset.model.materials);
  ResolveCollectionReferences(*this, meshes_, asset.model.meshes);
  ResolveCollectionReferences(*this, nodes_, asset.model.nodes);
  ResolveCollectionReferences(*this, textures_, asset.model.textures);
  ResolveCollectionReferences(*this, images_, asset.model.images);
  ResolveCollectionReferences(*this, skins_, asset.model.skins);
  ResolveCollectionReferences(*this, samplers_, asset.model.samplers);
  ResolveCollectionReferences(*this, cameras_, asset.model.cameras);
  ResolveCollectionReferences(*this, scenes_, asset.model.scenes);
  ResolveCollectionReferences(*this, lights_, asset.model.lights);
}

Model::~Model() = default;

const Accessors& Model::GetAccessors() const {
  return accessors_;
}

const Animations& Model::GetAnimations() const {
  return animations_;
}

const Buffers& Model::GetBuffers() const {
  return buffers_;
}

const BufferViews& Model::GetBufferViews() const {
  return bufferViews_;
}

const Materials& Model::GetMaterials() const {
  return materials_;
}

const Meshes& Model::GetMeshes() const {
  return meshes_;
}

const Nodes& Model::GetNodes() const {
  return nodes_;
}

const Textures& Model::GetTextures() const {
  return textures_;
}

const Images& Model::GetImages() const {
  return images_;
}

const Skins& Model::GetSkins() const {
  return skins_;
}

const Samplers& Model::GetSamplers() const {
  return samplers_;
}

const Cameras& Model::GetCameras() const {
  return cameras_;
}

const Scenes& Model::GetScenes() const {
  return scenes_;
}

const Lights& Model::GetLights() const {
  return lights_;
}

static void ArchiveRead(glm::vec3& ret, const std::vector<double>& input) {
  if (input.size() != 3) {
    return;
  }

  ret = {input[0], input[1], input[2]};
}

static void ArchiveRead(glm::mat4& ret, const std::vector<double>& input) {
  if (input.size() != 16) {
    return;
  }

  ret = {
      //
      input[0],  input[1],  input[2],  input[3],   //
      input[4],  input[5],  input[6],  input[7],   //
      input[8],  input[9],  input[10], input[11],  //
      input[12], input[13], input[14], input[15],  //
  };
}

static void ArchiveRead(glm::quat& ret, const std::vector<double>& input) {
  if (input.size() != 4) {
    return;
  }

  ret = {
      //
      static_cast<float>(input[0]), static_cast<float>(input[1]),
      static_cast<float>(input[2]), static_cast<float>(input[3]),  //
  };
}

template <class T>
auto BoundsCheckGet(const T& collection, int index) {
  using ReturnType = typename T::value_type;
  if (index < 0) {
    return ReturnType{};
  }

  const auto count = collection.size();

  if (index >= count) {
    return ReturnType{};
  }

  return collection[index];
}

static DataType DataTypeFromComponentType(uint32_t component_type) {
  switch (component_type) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      return DataType::kDataTypeByte;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      return DataType::kDataTypeUnsignedByte;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      return DataType::kDataTypeShort;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      return DataType::kDataTypeUnsignedShort;
    case TINYGLTF_COMPONENT_TYPE_INT:
      return DataType::kDataTypeInt;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      return DataType::kDataTypeUnsignedInt;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      return DataType::kDataTypeFloat;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      return DataType::kDataTypeDouble;
  }
  return DataType::kDataTypeUnknown;
}

static size_t SizeOfDataType(DataType type) {
  switch (type) {
    case DataType::kDataTypeUnknown:
      return 0;
    case DataType::kDataTypeByte:
      return 1;
    case DataType::kDataTypeUnsignedByte:
      return 1;
    case DataType::kDataTypeShort:
      return 2;
    case DataType::kDataTypeUnsignedShort:
      return 2;
    case DataType::kDataTypeInt:
      return 4;
    case DataType::kDataTypeUnsignedInt:
      return 4;
    case DataType::kDataTypeFloat:
      return 4;
    case DataType::kDataTypeDouble:
      return 8;
  }
  return 0;
}

// *****************************************************************************
// *** Accessor
// *****************************************************************************

Accessor::Accessor() = default;

Accessor::~Accessor() = default;

void Accessor::ReadFromArchive(const tinygltf::Accessor& accessor) {
  name_ = accessor.name;
  byte_offset_ = accessor.byteOffset;
  normalized_ = accessor.normalized;
  data_type_ = DataTypeFromComponentType(accessor.componentType);
  count_ = accessor.count;
  min_values_ = accessor.minValues;
  max_values_ = accessor.maxValues;
}

void Accessor::ResolveReferences(const Model& model,
                                 const tinygltf::Accessor& accessor) {
  buffer_view_ = BoundsCheckGet(model.bufferViews_, accessor.bufferView);
}

template <class T>
std::vector<T> CopyToVector(const void* ptr, size_t size) {
  auto typed_ptr = static_cast<const T*>(ptr);
  return {typed_ptr, typed_ptr + size};
}

template <class FinalType, class NativeType>
std::vector<FinalType> CopyAndConvertVector(const void* ptr, size_t size) {
  auto native_copy = CopyToVector<NativeType>(ptr, size);
  if constexpr (std::is_same<FinalType, NativeType>::value) {
    return native_copy;
  } else {
    return {native_copy.begin(), native_copy.end()};
  }
}

std::optional<std::vector<uint32_t>> Accessor::ReadIndexList() const {
  if (!buffer_view_) {
    P_ERROR << "Buffer view was nullptr.";
    return std::nullopt;
  }

  const auto stride = SizeOfDataType(data_type_);
  if (stride == 0) {
    P_ERROR << "Unknown data stride.";
    return std::nullopt;
  }

  auto buffer = buffer_view_->GetByteMapping(byte_offset_, stride * count_);

  if (!buffer.has_value()) {
    P_ERROR << "Accessor mapping was unavailable or out of bounds.";
    return std::nullopt;
  }

  const auto buffer_ptr = buffer.value();

  switch (data_type_) {
    case DataType::kDataTypeUnknown: {
      P_ERROR << "Unknown buffer view data type.";
      return std::nullopt;
    }
    case DataType::kDataTypeByte: {
      return CopyAndConvertVector<uint32_t, int8_t>(buffer_ptr, count_);
    }
    case DataType::kDataTypeUnsignedByte: {
      return CopyAndConvertVector<uint32_t, uint8_t>(buffer_ptr, count_);
    }
    case DataType::kDataTypeShort: {
      return CopyAndConvertVector<uint32_t, int16_t>(buffer_ptr, count_);
    }
    case DataType::kDataTypeUnsignedShort: {
      return CopyAndConvertVector<uint32_t, uint16_t>(buffer_ptr, count_);
    }
    case DataType::kDataTypeInt: {
      return CopyAndConvertVector<uint32_t, int32_t>(buffer_ptr, count_);
    }
    case DataType::kDataTypeUnsignedInt: {
      return CopyAndConvertVector<uint32_t, uint32_t>(buffer_ptr, count_);
    }
    case DataType::kDataTypeFloat: {
      return CopyAndConvertVector<uint32_t, float>(buffer_ptr, count_);
    }
    case DataType::kDataTypeDouble: {
      return CopyAndConvertVector<uint32_t, double>(buffer_ptr, count_);
    }
  }
  return std::nullopt;
}

// *****************************************************************************
// *** Animation
// *****************************************************************************

Animation::Animation() = default;

Animation::~Animation() = default;

void Animation::ReadFromArchive(const tinygltf::Animation& animation) {}

void Animation::ResolveReferences(const Model& model,
                                  const tinygltf::Animation& animation) {}

// *****************************************************************************
// *** Buffer
// *****************************************************************************

Buffer::Buffer() = default;

Buffer::~Buffer() = default;

void Buffer::ReadFromArchive(const tinygltf::Buffer& buffer) {
  name_ = buffer.name;
  data_ = CopyMapping(buffer.data.data(), buffer.data.size());
  uri_ = buffer.uri;
}

void Buffer::ResolveReferences(const Model& model,
                               const tinygltf::Buffer& buffer) {}

std::optional<const uint8_t*> Buffer::GetByteMapping(size_t byte_offset,
                                                     size_t byte_length) const {
  if (!data_ || data_->GetData() == nullptr) {
    P_ERROR << "Buffer data could not be resolved.";
    return std::nullopt;
  }

  if (byte_offset + byte_length > data_->GetSize()) {
    P_ERROR << "Buffer specified ranges with overruns.";
    return std::nullopt;
  }

  auto data_ptr = reinterpret_cast<const uint8_t*>(data_->GetData());

  return data_ptr + byte_offset;
}

// *****************************************************************************
// *** BufferView
// *****************************************************************************

BufferView::BufferView() = default;

BufferView::~BufferView() = default;

void BufferView::ReadFromArchive(const tinygltf::BufferView& view) {
  name_ = view.name;
  byte_offset_ = view.byteOffset;
  byte_length_ = view.byteLength;
  byte_stride_ = view.byteStride;
}

void BufferView::ResolveReferences(const Model& model,
                                   const tinygltf::BufferView& view) {
  buffer_ = BoundsCheckGet(model.buffers_, view.buffer);
}

std::optional<const uint8_t*> BufferView::GetByteMapping(
    size_t accessor_offset,
    size_t accessor_length) const {
  if (byte_stride_ != 0) {
    // TODO: This must be fixed in the Accessor.
    P_ERROR << "This renderer cannot work with non zero byte strides. The data "
               "must be tightly packed.";
    return std::nullopt;
  }

  if (!buffer_) {
    P_ERROR << "Buffer was not present.";
    return std::nullopt;
  }

  auto buffer_ptr = buffer_->GetByteMapping(byte_offset_, byte_length_);

  if (!buffer_ptr.has_value()) {
    P_ERROR << "Could not map the buffer from the buffer view.";
    return std::nullopt;
  }

  if (accessor_offset + accessor_length > byte_length_) {
    P_ERROR << "Buffer view specified ranges with overruns.";
    return std::nullopt;
  }

  return buffer_ptr.value() + accessor_offset;
}

// *****************************************************************************
// *** Material
// *****************************************************************************

Material::Material() = default;

Material::~Material() = default;

void Material::ReadFromArchive(const tinygltf::Material& material) {}

void Material::ResolveReferences(const Model& model,
                                 const tinygltf::Material& material) {}

// *****************************************************************************
// *** Primitive
// *****************************************************************************

Primitive::Primitive() = default;

Primitive::~Primitive() = default;

void Primitive::ReadFromArchive(const tinygltf::Primitive& primitive) {
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

void Primitive::ResolveReferences(const Model& model,
                                  const tinygltf::Primitive& primitive) {
  for (const auto& attribute : primitive.attributes) {
    if (attribute.first.empty()) {
      continue;
    }
    auto accessor = BoundsCheckGet(model.accessors_, attribute.second);
    if (!accessor) {
      continue;
    }
    attributes_[attribute.first] = accessor;
  }
  material_ = BoundsCheckGet(model.materials_, primitive.material);
  indices_ = BoundsCheckGet(model.accessors_, primitive.indices);
}

std::shared_ptr<Accessor> Primitive::GetPositionAttribute() const {
  auto found = attributes_.find("POSITION");
  if (found == attributes_.end()) {
    return nullptr;
  }
  return found->second;
}

// *****************************************************************************
// *** Mesh
// *****************************************************************************

Mesh::Mesh() = default;

Mesh::~Mesh() = default;

void Mesh::ReadFromArchive(const tinygltf::Mesh& mesh) {
  name_ = mesh.name;
  primitives_ = Inflate<Primitive, tinygltf::Primitive>(mesh.primitives);
  weights_ = mesh.weights;
}

void Mesh::ResolveReferences(const Model& model, const tinygltf::Mesh& mesh) {
  ResolveCollectionReferences(model, primitives_, mesh.primitives);
}

const std::string& Mesh::GetName() const {
  return name_;
}

const std::vector<std::shared_ptr<Primitive>>& Mesh::GetPrimitives() const {
  return primitives_;
}

const std::vector<double>& Mesh::GetWeights() const {
  return weights_;
}

// *****************************************************************************
// *** Node
// *****************************************************************************

Node::Node() = default;

Node::~Node() = default;

void Node::ReadFromArchive(const tinygltf::Node& node) {
  name_ = node.name;
  ArchiveRead(rotation_, node.rotation);
  ArchiveRead(scale_, node.rotation);
  ArchiveRead(translation_, node.rotation);
  ArchiveRead(matrix_, node.rotation);
  weights_ = node.weights;
}

void Node::ResolveReferences(const Model& model, const tinygltf::Node& node) {
  camera_ = BoundsCheckGet(model.cameras_, node.camera);
  skin_ = BoundsCheckGet(model.skins_, node.skin);
  mesh_ = BoundsCheckGet(model.meshes_, node.mesh);
  for (const auto& child : node.children) {
    if (auto node = BoundsCheckGet(model.nodes_, child)) {
      children_.emplace_back(std::move(node));
    }
  }
}

const std::string& Node::GetName() const {
  return name_;
}

const std::shared_ptr<Camera>& Node::GetCamera() const {
  return camera_;
}

const std::shared_ptr<Skin>& Node::GetSkin() const {
  return skin_;
}

const std::shared_ptr<Mesh>& Node::GetMesh() const {
  return mesh_;
}

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const {
  return children_;
}

const glm::quat& Node::GetRotation() const {
  return rotation_;
}

const glm::vec3& Node::GetScale() const {
  return scale_;
}

const glm::vec3& Node::GetTranslation() const {
  return translation_;
}

const glm::mat4& Node::GetMatrix() const {
  return matrix_;
}

const std::vector<double>& Node::GetWeights() const {
  return weights_;
}

// *****************************************************************************
// *** Texture
// *****************************************************************************

Texture::Texture() = default;

Texture::~Texture() = default;

void Texture::ReadFromArchive(const tinygltf::Texture& texture) {
  name_ = texture.name;
}

void Texture::ResolveReferences(const Model& model,
                                const tinygltf::Texture& texture) {
  sampler_ = BoundsCheckGet(model.samplers_, texture.sampler);
  source_ = BoundsCheckGet(model.images_, texture.source);
}

// *****************************************************************************
// *** Image
// *****************************************************************************

Image::Image() = default;

Image::~Image() = default;

void Image::ReadFromArchive(const tinygltf::Image& image) {
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

void Image::ResolveReferences(const Model& model,
                              const tinygltf::Image& image) {
  buffer_view_ = BoundsCheckGet(model.bufferViews_, image.bufferView);
}

// *****************************************************************************
// *** Skin
// *****************************************************************************

Skin::Skin() = default;

Skin::~Skin() = default;

void Skin::ReadFromArchive(const tinygltf::Skin& skin) {}

void Skin::ResolveReferences(const Model& model, const tinygltf::Skin& skin) {}

// *****************************************************************************
// *** Sampler
// *****************************************************************************

static std::pair<vk::Filter, vk::SamplerMipmapMode> ParseVkFilter(int filter) {
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

static vk::SamplerAddressMode ParseVkSamplerAddressMode(int mode) {
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

Sampler::Sampler() = default;

Sampler::~Sampler() = default;

void Sampler::ReadFromArchive(const tinygltf::Sampler& sampler) {
  name_ = sampler.name;
  // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/schema/sampler.schema.json
  std::tie(mag_filter_, std::ignore) = ParseVkFilter(sampler.magFilter);
  std::tie(min_filter_, mipmap_mode_) = ParseVkFilter(sampler.minFilter);
  wrap_s_ = ParseVkSamplerAddressMode(sampler.wrapS);
  wrap_t_ = ParseVkSamplerAddressMode(sampler.wrapT);
  wrap_r_ = ParseVkSamplerAddressMode(sampler.wrapR);
}

void Sampler::ResolveReferences(const Model& model,
                                const tinygltf::Sampler& sampler) {}

// *****************************************************************************
// *** Camera
// *****************************************************************************

Camera::Camera() = default;

Camera::~Camera() = default;

void Camera::ReadFromArchive(const tinygltf::Camera& camera) {
  name_ = camera.name;

  if (camera.type == "perspective") {
    projection_ = PerspectiveCamera{
        .aspect_ratio = std::max<double>(0.0, camera.perspective.aspectRatio),
        .y_fov = std::max<double>(0.0, camera.perspective.yfov),
        .z_far = std::max<double>(0.0, camera.perspective.zfar),
        .z_near = std::max<double>(0.0, camera.perspective.znear),
    };
  } else if (camera.type == "orthographic") {
    projection_ = OrthographicCamera{
        .x_mag = camera.orthographic.xmag,
        .y_mag = camera.orthographic.ymag,
        .z_far = camera.orthographic.zfar,
        .z_near = camera.orthographic.znear,
    };
  }
}

void Camera::ResolveReferences(const Model& model,
                               const tinygltf::Camera& camera) {}

// *****************************************************************************
// *** Scene
// *****************************************************************************

Scene::Scene() = default;

Scene::~Scene() = default;

const Nodes& Scene::GetNodes() const {
  return nodes_;
}

void Scene::ReadFromArchive(const tinygltf::Scene& scene) {
  name_ = scene.name;
}

void Scene::ResolveReferences(const Model& model,
                              const tinygltf::Scene& scene) {
  for (const auto& node_index : scene.nodes) {
    if (auto node = BoundsCheckGet(model.nodes_, node_index)) {
      nodes_.emplace_back(std::move(node));
    }
  }
}

// *****************************************************************************
// *** Light
// *****************************************************************************

Light::Light() = default;

Light::~Light() = default;

void Light::ReadFromArchive(const tinygltf::Light& light) {}

void Light::ResolveReferences(const Model& model,
                              const tinygltf::Light& light) {}

}  // namespace model
}  // namespace pixel
