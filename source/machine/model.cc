#include "model.h"

#include <type_traits>

#include "model_draw_data.h"

namespace pixel {
namespace model {

template <class ResourceType, class GLTFResourceType>
static std::shared_ptr<ResourceType> Inflate(const GLTFResourceType& item) {
  auto resource = std::make_shared<ResourceType>();
  resource->ReadFromArchive(item);
  return resource;
}

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
    P_ERROR << "Attempted to access resource at index out-of-bounds.";
    return ReturnType{};
  }

  return collection[index];
}

static DataType DataTypeFromDataType(uint32_t data_type) {
  switch (data_type) {
    case TINYGLTF_TYPE_VEC2:
      return DataType::kDataTypeVec2;
    case TINYGLTF_TYPE_VEC3:
      return DataType::kDataTypeVec3;
    case TINYGLTF_TYPE_VEC4:
      return DataType::kDataTypeVec4;
    case TINYGLTF_TYPE_MAT2:
      return DataType::kDataTypeMat2;
    case TINYGLTF_TYPE_MAT3:
      return DataType::kDataTypeMat3;
    case TINYGLTF_TYPE_MAT4:
      return DataType::kDataTypeMat4;
    case TINYGLTF_TYPE_SCALAR:
      return DataType::kDataTypeScalar;
    case TINYGLTF_TYPE_VECTOR:
      return DataType::kDataTypeVector;
    case TINYGLTF_TYPE_MATRIX:
      return DataType::kDataTypeMatrix;
  }
  return DataType::kDataTypeUnknown;
}

static size_t NumberOfComponentsInDataType(DataType type) {
  switch (type) {
    case DataType::kDataTypeUnknown:
      return 0;
    case DataType::kDataTypeVec2:
      return 2;
    case DataType::kDataTypeVec3:
      return 3;
    case DataType::kDataTypeVec4:
      return 4;
    case DataType::kDataTypeMat2:
      return 4;
    case DataType::kDataTypeMat3:
      return 9;
    case DataType::kDataTypeMat4:
      return 16;
    case DataType::kDataTypeScalar:
      return 1;
    case DataType::kDataTypeVector:
      // TODO: What is this?
      return 0;
    case DataType::kDataTypeMatrix:
      return 16;
  }
  return 0;
}

static ComponentType ComponentTypeFromComponentType(uint32_t component_type) {
  switch (component_type) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      return ComponentType::kComponentTypeByte;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      return ComponentType::kComponentTypeUnsignedByte;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      return ComponentType::kComponentTypeShort;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      return ComponentType::kComponentTypeUnsignedShort;
    case TINYGLTF_COMPONENT_TYPE_INT:
      return ComponentType::kComponentTypeInt;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      return ComponentType::kComponentTypeUnsignedInt;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      return ComponentType::kComponentTypeFloat;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      return ComponentType::kComponentTypeDouble;
  }
  return ComponentType::kComponentTypeUnknown;
}

static size_t SizeOfComponentType(ComponentType type) {
  switch (type) {
    case ComponentType::kComponentTypeUnknown:
      return 0;
    case ComponentType::kComponentTypeByte:
      return 1;
    case ComponentType::kComponentTypeUnsignedByte:
      return 1;
    case ComponentType::kComponentTypeShort:
      return 2;
    case ComponentType::kComponentTypeUnsignedShort:
      return 2;
    case ComponentType::kComponentTypeInt:
      return 4;
    case ComponentType::kComponentTypeUnsignedInt:
      return 4;
    case ComponentType::kComponentTypeFloat:
      return 4;
    case ComponentType::kComponentTypeDouble:
      return 8;
  }
  return 0;
}

std::unique_ptr<ModelDrawData> Model::CreateDrawData(
    std::string debug_name) const {
  auto draw_data = std::make_unique<ModelDrawData>(std::move(debug_name));

  TransformationStack stack;
  for (const auto& scene : scenes_) {
    if (!scene->CollectDrawData(*draw_data, stack)) {
      return {};
    }
  }
  return draw_data;
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
  component_type_ = ComponentTypeFromComponentType(accessor.componentType);
  data_type_ = DataTypeFromDataType(accessor.type);
  count_ = accessor.count;
  min_values_ = accessor.minValues;
  max_values_ = accessor.maxValues;
}

void Accessor::ResolveReferences(const Model& model,
                                 const tinygltf::Accessor& accessor) {
  buffer_view_ = BoundsCheckGet(model.bufferViews_, accessor.bufferView);
}

template <class T>
static std::vector<T> CopyToVector(const void* ptr,
                                   size_t size,
                                   size_t stride,
                                   size_t component_count) {
  if (stride == 0) {
    // Data is tightly packed. Simple ::memcpy is sufficient.
    auto typed_ptr = static_cast<const T*>(ptr);
    return {typed_ptr, typed_ptr + size};
  }

  std::vector<T> return_vector;
  return_vector.reserve(size);
  auto byte_ptr = reinterpret_cast<const uint8_t*>(ptr);

  for (size_t i = 0; i < size; i += component_count) {
    for (size_t j = 0; j < component_count; j++) {
      return_vector.push_back({});
      ::memcpy(&return_vector.back(), byte_ptr + (j * sizeof(T)), sizeof(T));
    }
    byte_ptr += stride;
  }

  P_ASSERT(return_vector.size() == size);
  return return_vector;
}

template <class FinalType, class NativeType>
static std::vector<FinalType> CopyAndConvertVector(const void* ptr,
                                                   size_t size,
                                                   size_t stride,
                                                   size_t component_count) {
  auto native_copy =
      CopyToVector<NativeType>(ptr, size, stride, component_count);
  if constexpr (std::is_same<FinalType, NativeType>::value) {
    return native_copy;
  } else {
    return {native_copy.begin(), native_copy.end()};
  }
}

template <class T>
static std::optional<std::vector<T>> ReadTypedList(
    const BufferView* buffer_view,
    DataType data_type,
    ComponentType component_type,
    size_t byte_offset,
    size_t items_count) {
  if (items_count == 0) {
    P_ERROR << "Items count was zero.";
    return std::nullopt;
  }

  if (!buffer_view) {
    P_ERROR << "Buffer view was nullptr.";
    return std::nullopt;
  }

  const auto component_stride = SizeOfComponentType(component_type);
  if (component_stride == 0) {
    P_ERROR << "Unknown data stride.";
    return std::nullopt;
  }

  const auto components_count = NumberOfComponentsInDataType(data_type);

  if (components_count == 0) {
    P_ERROR << "Unknown number of components in data type.";
    return std::nullopt;
  }

  const auto elements_count = items_count * components_count;

  const auto buffer_view_stride = buffer_view->GetStride();

  if (buffer_view_stride % 4 != 0) {
    P_ERROR << "Buffer view stride was not a multiple of 4.";
  }

  // TODO: This bounds check might not be sufficient to account for the stride.
  auto buffer = buffer_view->GetByteMapping(byte_offset,
                                            component_stride * elements_count);

  if (!buffer.has_value()) {
    P_ERROR << "Accessor mapping was unavailable or out of bounds.";
    return std::nullopt;
  }

  const auto buffer_ptr = buffer.value();

  switch (component_type) {
    case ComponentType::kComponentTypeUnknown: {
      P_ERROR << "Unknown buffer view data type.";
      return std::nullopt;
    }
    case ComponentType::kComponentTypeByte: {
      return CopyAndConvertVector<T, int8_t>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
    case ComponentType::kComponentTypeUnsignedByte: {
      return CopyAndConvertVector<T, uint8_t>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
    case ComponentType::kComponentTypeShort: {
      return CopyAndConvertVector<T, int16_t>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
    case ComponentType::kComponentTypeUnsignedShort: {
      return CopyAndConvertVector<T, uint16_t>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
    case ComponentType::kComponentTypeInt: {
      return CopyAndConvertVector<T, int32_t>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
    case ComponentType::kComponentTypeUnsignedInt: {
      return CopyAndConvertVector<T, uint32_t>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
    case ComponentType::kComponentTypeFloat: {
      return CopyAndConvertVector<T, float>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
    case ComponentType::kComponentTypeDouble: {
      return CopyAndConvertVector<T, double>(
          buffer_ptr, elements_count, buffer_view_stride, components_count);
    }
  }
  return std::nullopt;
}

std::optional<std::vector<uint32_t>> Accessor::ReadIndexList() const {
  const auto component_count = NumberOfComponentsInDataType(data_type_);
  if (component_count != 1) {
    P_ERROR << "Incorrect number of components.";
    return std::nullopt;
  }

  return ReadTypedList<uint32_t>(buffer_view_.get(),  //
                                 data_type_,          //
                                 component_type_,     //
                                 byte_offset_,        //
                                 count_               //
  );
}

std::optional<std::vector<glm::vec3>> Accessor::ReadVec3List() const {
  const auto component_count = NumberOfComponentsInDataType(data_type_);
  if (component_count != 3) {
    P_ERROR << "Incorrect number of components.";
    return std::nullopt;
  }

  auto list = ReadTypedList<float>(buffer_view_.get(),  //
                                   data_type_,          //
                                   component_type_,     //
                                   byte_offset_,        //
                                   count_               //
  );

  if (!list.has_value()) {
    P_ERROR << "Could not read data list.";
    return std::nullopt;
  }

  const auto& float_list = list.value();

  std::vector<glm::vec3> vec_list;
  vec_list.reserve(float_list.size() / 3);

  for (size_t i = 0; i < float_list.size() / 3; i++) {
    vec_list.emplace_back(glm::make_vec3(float_list.data() + (i * 3)));
  }

  return vec_list;
}

// TODO: Dry this up. This is a straight up copy of ReadVec3List.
std::optional<std::vector<glm::vec2>> Accessor::ReadVec2List() const {
  const auto component_count = NumberOfComponentsInDataType(data_type_);
  if (component_count != 2) {
    P_ERROR << "Incorrect number of components.";
    return std::nullopt;
  }

  auto list = ReadTypedList<float>(buffer_view_.get(),  //
                                   data_type_,          //
                                   component_type_,     //
                                   byte_offset_,        //
                                   count_               //
  );

  if (!list.has_value()) {
    P_ERROR << "Could not read data list.";
    return std::nullopt;
  }

  const auto& float_list = list.value();

  std::vector<glm::vec2> vec_list;
  vec_list.reserve(float_list.size() / 2);

  for (size_t i = 0; i < float_list.size() / 2; i++) {
    vec_list.emplace_back(glm::make_vec2(float_list.data() + (i * 2)));
  }

  return vec_list;
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

size_t BufferView::GetStride() const {
  return byte_stride_;
}

// *****************************************************************************
// *** TextureInfo
// *****************************************************************************

TextureInfo::TextureInfo() = default;

TextureInfo::~TextureInfo() = default;

void TextureInfo::ReadFromArchive(const tinygltf::TextureInfo& texture) {
  texture_coord_index_ = texture.texCoord;
}

void TextureInfo::ResolveReferences(const Model& model,
                                    const tinygltf::TextureInfo& texture) {
  texture_ = BoundsCheckGet(model.textures_, texture.index);
}

bool TextureInfo::CollectDrawData(TextureType type,
                                  ModelDrawCallBuilder& draw_call) const {
  if (texture_) {
    if (!texture_->CollectDrawData(type, draw_call)) {
      return false;
    }
  }

  return true;
}
// *****************************************************************************
// *** NormalTextureInfo
// *****************************************************************************

NormalTextureInfo::NormalTextureInfo() = default;

NormalTextureInfo::~NormalTextureInfo() = default;

void NormalTextureInfo::ReadFromArchive(
    const tinygltf::NormalTextureInfo& normal) {
  texture_coord_index_ = normal.texCoord;
  scale_ = normal.scale;
}

void NormalTextureInfo::ResolveReferences(
    const Model& model,
    const tinygltf::NormalTextureInfo& normal) {
  texture_ = BoundsCheckGet(model.textures_, normal.index);
}

// *****************************************************************************
// *** OcclusionTextureInfo
// *****************************************************************************

OcclusionTextureInfo::OcclusionTextureInfo() = default;

OcclusionTextureInfo::~OcclusionTextureInfo() = default;

void OcclusionTextureInfo::ReadFromArchive(
    const tinygltf::OcclusionTextureInfo& occlusion) {
  texture_coord_index_ = occlusion.texCoord;
  strength_ = occlusion.strength;
}

void OcclusionTextureInfo::ResolveReferences(
    const Model& model,
    const tinygltf::OcclusionTextureInfo& occlusion) {
  texture_ = BoundsCheckGet(model.textures_, occlusion.index);
}

// *****************************************************************************
// *** PBRMetallicRoughness
// *****************************************************************************

PBRMetallicRoughness::PBRMetallicRoughness() = default;

PBRMetallicRoughness::~PBRMetallicRoughness() = default;

void PBRMetallicRoughness::ReadFromArchive(
    const tinygltf::PbrMetallicRoughness& roughness) {
  base_color_factor_ = roughness.baseColorFactor;
  base_color_texture_ =
      Inflate<TextureInfo, tinygltf::TextureInfo>(roughness.baseColorTexture);
  metallic_factor_ = roughness.metallicFactor;
  roughness_factor_ = roughness.roughnessFactor;
  metallic_roughness_texture_ = Inflate<TextureInfo, tinygltf::TextureInfo>(
      roughness.metallicRoughnessTexture);
}

void PBRMetallicRoughness::ResolveReferences(
    const Model& model,
    const tinygltf::PbrMetallicRoughness& roughness) {
  base_color_texture_->ResolveReferences(model, roughness.baseColorTexture);
  metallic_roughness_texture_->ResolveReferences(
      model, roughness.metallicRoughnessTexture);
}

bool PBRMetallicRoughness::CollectDrawData(
    ModelDrawData& data,
    ModelDrawCallBuilder& draw_call) const {
  if (base_color_texture_) {
    base_color_texture_->CollectDrawData(TextureType::kTextureTypeBaseColor,
                                         draw_call);
  }

  return true;
}

// *****************************************************************************
// *** Material
// *****************************************************************************

Material::Material() = default;

Material::~Material() = default;

void Material::ReadFromArchive(const tinygltf::Material& material) {
  name_ = material.name;
  emissive_factor_ = material.emissiveFactor;
  is_opaque_ = material.alphaMode == "OPAQUE";
  alpha_cutoff_ = material.alphaCutoff;
  double_sided_ = material.doubleSided;
  pbr_metallic_roughness_ =
      Inflate<PBRMetallicRoughness, tinygltf::PbrMetallicRoughness>(
          material.pbrMetallicRoughness);
  occlusion_texture_ =
      Inflate<OcclusionTextureInfo, tinygltf::OcclusionTextureInfo>(
          material.occlusionTexture);
  emissive_texture_ =
      Inflate<TextureInfo, tinygltf::TextureInfo>(material.emissiveTexture);
}

void Material::ResolveReferences(const Model& model,
                                 const tinygltf::Material& material) {
  pbr_metallic_roughness_->ResolveReferences(model,
                                             material.pbrMetallicRoughness);
  occlusion_texture_->ResolveReferences(model, material.occlusionTexture);
  emissive_texture_->ResolveReferences(model, material.emissiveTexture);
}

bool Material::CollectDrawData(ModelDrawData& data,
                               ModelDrawCallBuilder& draw_call,
                               const TransformationStack& stack) const {
  if (pbr_metallic_roughness_) {
    if (!pbr_metallic_roughness_->CollectDrawData(data, draw_call)) {
      return false;
    }
  }

  return true;
}

// *****************************************************************************
// *** Primitive
// *****************************************************************************

Primitive::Primitive() = default;

Primitive::~Primitive() = default;

void Primitive::ReadFromArchive(const tinygltf::Primitive& primitive) {
  switch (primitive.mode) {
    case TINYGLTF_MODE_POINTS:
      mode_ = vk::PrimitiveTopology::ePointList;
      break;
    case TINYGLTF_MODE_LINE:
      mode_ = vk::PrimitiveTopology::eLineList;
      break;
    case TINYGLTF_MODE_LINE_LOOP:
      // TODO: WTF is this?
      // mode_ = vk::PrimitiveTopology::elineloop;
      break;
    case TINYGLTF_MODE_LINE_STRIP:
      mode_ = vk::PrimitiveTopology::eLineStrip;
      break;
    case TINYGLTF_MODE_TRIANGLES:
      mode_ = vk::PrimitiveTopology::eTriangleList;
      break;
    case TINYGLTF_MODE_TRIANGLE_STRIP:
      mode_ = vk::PrimitiveTopology::eTriangleStrip;
      break;
    case TINYGLTF_MODE_TRIANGLE_FAN:
      mode_ = vk::PrimitiveTopology::eTriangleFan;
      break;
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

std::shared_ptr<Accessor> Primitive::GetTextureCoordAttribute() const {
  auto found = attributes_.find("TEXCOORD_0");
  if (found == attributes_.end()) {
    return nullptr;
  }
  return found->second;
}

std::shared_ptr<Accessor> Primitive::GetNormalAttribute() const {
  auto found = attributes_.find("NORMAL");
  if (found == attributes_.end()) {
    return nullptr;
  }
  return found->second;
}

static std::vector<glm::vec3> VectorWithAppliedTransformations(
    const TransformationStack& stack,
    const std::vector<glm::vec3>& vec) {
  std::vector<glm::vec3> result;
  result.reserve(vec.size());
  for (const auto& item : vec) {
    const auto transformed = stack.transformation * glm::vec4(item, 1.0);
    result.push_back(glm::vec3{transformed.x, transformed.y, transformed.z});
  }
  return result;
}

bool Primitive::CollectDrawData(ModelDrawData& draw_data,
                                const TransformationStack& stack) const {
  ModelDrawCallBuilder draw_call_builder;

  draw_call_builder.SetTopology(mode_);

  // Outside scope of vertices because we will be performing an index bounds
  // check later.
  std::vector<glm::vec3> positions;

  // Collect vertices.
  {
    std::vector<glm::vec2> texture_coords;
    std::vector<glm::vec3> normals;

    if (auto position = GetPositionAttribute()) {
      auto position_data = position->ReadVec3List();
      if (position_data.has_value()) {
        positions = VectorWithAppliedTransformations(
            stack, std::move(position_data.value()));
      }
    }

    if (auto normal = GetNormalAttribute()) {
      auto normal_data = normal->ReadVec3List();
      if (normal_data.has_value()) {
        normals = std::move(normal_data.value());
      }
    }

    if (auto texture_coord = GetTextureCoordAttribute()) {
      auto texture_coord_data = texture_coord->ReadVec2List();
      if (texture_coord_data.has_value()) {
        texture_coords = std::move(texture_coord_data.value());
      }
    }

    const bool has_normals = normals.size() == positions.size();
    const bool has_texture_coords = texture_coords.size() == positions.size();

    std::vector<shaders::model_renderer::Vertex> vertices;
    vertices.reserve(positions.size());

    for (size_t i = 0, count = positions.size(); i < count; i++) {
      vertices.push_back(shaders::model_renderer::Vertex{
          positions[i],                                // position
          has_normals ? normals[i] : glm::vec3{0.0f},  // normal
          has_texture_coords ? texture_coords[i]
                             : glm::vec2{0.0f},  // texture coords
      });
    }

    draw_call_builder.SetVertices(std::move(vertices));
  }

  // Collect indices.
  if (indices_) {
    auto index_data = indices_->ReadIndexList();
    if (index_data.has_value()) {
      auto indices = std::move(index_data.value());

      for (const auto& index : indices) {
        if (index >= positions.size()) {
          P_ERROR << "Index specified vertex position that was out of bounds.";
          return false;
        }
      }

      draw_call_builder.SetIndices(std::move(indices));
    }
  }

  // Collect materials (samplers, etc.)
  if (material_) {
    if (!material_->CollectDrawData(draw_data, draw_call_builder, stack)) {
      return false;
    }
  }

  draw_data.AddDrawCall(draw_call_builder.CreateDrawCall());

  return true;
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

bool Mesh::CollectDrawData(ModelDrawData& data,
                           const TransformationStack& stack) const {
  for (const auto& primitive : primitives_) {
    if (!primitive->CollectDrawData(data, stack)) {
      return false;
    }
  }
  return true;
}

// *****************************************************************************
// *** Node
// *****************************************************************************

Node::Node() = default;

Node::~Node() = default;

void Node::ReadFromArchive(const tinygltf::Node& node) {
  name_ = node.name;
  ArchiveRead(rotation_, node.rotation);
  ArchiveRead(scale_, node.scale);
  ArchiveRead(translation_, node.translation);
  ArchiveRead(matrix_, node.matrix);
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

bool Node::CollectDrawData(ModelDrawData& data,
                           TransformationStack stack) const {
  stack.transformation *= GetTransformation();

  if (mesh_) {
    if (!mesh_->CollectDrawData(data, stack)) {
      return false;
    }
  }

  for (const auto& child : children_) {
    if (!child->CollectDrawData(data, stack)) {
      return false;
    }
  }

  return true;
}

glm::mat4 Node::GetTransformation() const {
  auto scale = glm::scale(glm::identity<glm::mat4>(), scale_);
  auto rotate = glm::mat4(rotation_);
  auto translate = glm::translate(glm::identity<glm::mat4>(), translation_);

  return translate * rotate * scale * matrix_;
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

bool Texture::CollectDrawData(TextureType type,
                              ModelDrawCallBuilder& draw_call) const {
  draw_call.SetTexture(type, source_, sampler_);
  return true;
}

// *****************************************************************************
// *** Image
// *****************************************************************************

Image::Image() = default;

Image::~Image() = default;

static ScalarFormat PixelTypeToScalarFormat(int pixel_type) {
  switch (pixel_type) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      return ScalarFormat::kByte;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      return ScalarFormat::kUnsignedByte;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      return ScalarFormat::kShort;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      return ScalarFormat::kUnsignedShort;
    case TINYGLTF_COMPONENT_TYPE_INT:
      return ScalarFormat::kInt;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      return ScalarFormat::kUnsignedInt;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      return ScalarFormat::kFloat;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      return ScalarFormat::kDouble;
  }
  return ScalarFormat::kUnknown;
}

void Image::ReadFromArchive(const tinygltf::Image& image) {
  name_ = image.name;
  width_ = std::max<int>(0u, image.width);
  height_ = std::max<int>(0u, image.height);
  components_ = std::max<int>(0u, image.component);
  bits_per_component_ = std::max<int>(0u, image.bits);
  component_format_ = PixelTypeToScalarFormat(image.pixel_type);
  decompressed_image_ = CopyMapping(image.image.data(), image.image.size());
  mime_type_ = image.mimeType;
  uri_ = image.uri;
}

void Image::ResolveReferences(const Model& model,
                              const tinygltf::Image& image) {
  buffer_view_ = BoundsCheckGet(model.bufferViews_, image.bufferView);
}

std::unique_ptr<pixel::ImageView> Image::CreateImageView(
    const RenderingContext& context) const {
  if (width_ == 0 || height_ == 0) {
    return nullptr;
  }

  if (!decompressed_image_ || decompressed_image_->GetData() == nullptr) {
    return nullptr;
  }

  auto format = context.GetOptimalSampledImageFormat(components_,          //
                                                     bits_per_component_,  //
                                                     component_format_     //
  );

  if (!format.has_value()) {
    return nullptr;
  }

  vk::ImageCreateInfo image_create_info = {
      {},                  // flags
      vk::ImageType::e2D,  // image type
      format.value(),      // image format
      vk::Extent3D{static_cast<uint32_t>(width_),
                   static_cast<uint32_t>(height_), 1u},  // extents
      1u,                                                // mip levels
      1u,                                                // array layers
      vk::SampleCountFlagBits::e1,                       // samples
      vk::ImageTiling::eOptimal,                         // tiling
      vk::ImageUsageFlagBits::eSampled |
          vk::ImageUsageFlagBits::eTransferDst,  // usage
      vk::SharingMode::eExclusive,               // sharing (we are not)
      0u,                          // queue family index count (for sharing)
      nullptr,                     // queue families (for sharing)
      vk::ImageLayout::eUndefined  // initial layout
  };

  // TODO: Figure out device synchronization.

  auto image = context.GetMemoryAllocator().CreateDeviceLocalImageCopy(
      image_create_info,                              //
      decompressed_image_->GetData(),                 //
      decompressed_image_->GetSize(),                 //
      context.GetTransferCommandPool(),               //
      name_.empty() ? "Model Image" : name_.c_str(),  //
      nullptr,                                        // wait semaphores
      nullptr,                                        //  wait stages
      nullptr,                                        // signal semaphores
      nullptr                                         // on done
  );

  if (!image) {
    return nullptr;
  }

  vk::ImageViewCreateInfo image_view_create_info = {
      {},                      // flags
      image->image,            // image
      vk::ImageViewType::e2D,  // type
      format.value(),          // format
      {},                      // component mapping
      vk::ImageSubresourceRange{
          vk::ImageAspectFlagBits::eColor,  // aspect
          0u,                               // base mip level
          1u,                               // level count
          0u,                               // base array layer
          1u,                               // layer count
      },                                    // subresource range
  };

  auto image_view = UnwrapResult(
      context.GetDevice().createImageViewUnique(image_view_create_info));

  if (!image_view) {
    return nullptr;
  }

  SetDebugName(context.GetDevice(), image_view.get(),
               name_.empty() ? "Model Image View" : name_.c_str());

  return std::make_unique<pixel::ImageView>(std::move(image),
                                            std::move(image_view));
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
                                const tinygltf::Sampler& sampler) {
  // Nothing to do.
}

vk::UniqueSampler Sampler::CreateSampler(
    const RenderingContext& context) const {
  vk::SamplerCreateInfo sampler_info = {
      {},                                       // flags
      mag_filter_,                              // mag filter
      min_filter_,                              // min filter
      mipmap_mode_,                             // mip map mode
      wrap_s_,                                  // address mode U
      wrap_t_,                                  // address mode V
      wrap_r_,                                  // address mode W
      0.0f,                                     // mip LOD bias
      context.GetFeatures().samplerAnisotropy,  // enable anisotropic filtering
      16.0f,                                    // max anisotropy
      false,                                    // compare enable
      vk::CompareOp::eNever,                    // copare op
      0.0f,                                     // min LOD
      0.0f,                                     // max LOD
      vk::BorderColor::eFloatTransparentBlack,  // border color
      false,                                    // unnormalized coordinates
  };

  auto sampler =
      UnwrapResult(context.GetDevice().createSamplerUnique(sampler_info));
  if (!sampler) {
    return {};
  }

  if (!name_.empty()) {
    SetDebugName(context.GetDevice(), sampler.get(), name_.c_str());
  }

  return sampler;
}

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

bool Scene::CollectDrawData(ModelDrawData& data,
                            const TransformationStack& stack) const {
  for (const auto& node : nodes_) {
    if (!node->CollectDrawData(data, stack)) {
      return false;
    }
  }
  return true;
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
