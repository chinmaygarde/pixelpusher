#include "model_draw_data.h"

#include "pipeline_builder.h"
#include "pipeline_layout.h"
#include "string_utils.h"

namespace pixel {
namespace model {

ModelDrawCall::ModelDrawCall(
    vk::PrimitiveTopology topology,
    std::vector<uint32_t> indices,
    std::vector<pixel::shaders::model_renderer::Vertex> vertices,
    ModelTextureMap textures)
    : topology_(topology),
      indices_(std::move(indices)),
      vertices_(std::move(vertices)),
      textures_(std::move(textures)) {}

ModelDrawCall::~ModelDrawCall() = default;

// *****************************************************************************
// *** ModelDrawCallBuilder
// *****************************************************************************

ModelDrawCallBuilder::ModelDrawCallBuilder() = default;

ModelDrawCallBuilder::~ModelDrawCallBuilder() = default;

ModelDrawCallBuilder& ModelDrawCallBuilder::SetTopology(
    vk::PrimitiveTopology topology) {
  topology_ = topology;
  return *this;
}

ModelDrawCallBuilder& ModelDrawCallBuilder::SetIndices(
    std::vector<uint32_t> indices) {
  indices_ = std::move(indices);
  return *this;
}

ModelDrawCallBuilder& ModelDrawCallBuilder::SetVertices(
    std::vector<pixel::shaders::model_renderer::Vertex> vertices) {
  vertices_ = std::move(vertices);
  return *this;
}

ModelDrawCallBuilder& ModelDrawCallBuilder::SetTexture(
    TextureType type,
    std::shared_ptr<Image> image,
    std::shared_ptr<Sampler> sampler) {
  textures_[type] = std::make_pair(std::move(image), std::move(sampler));
  return *this;
}

std::shared_ptr<ModelDrawCall> ModelDrawCallBuilder::CreateDrawCall() {
  return std::make_shared<ModelDrawCall>(topology_, std::move(indices_),
                                         std::move(vertices_),
                                         std::move(textures_));
}

// *****************************************************************************
// *** ModelDeviceContext
// *****************************************************************************

ModelDeviceContext::ModelDeviceContext(
    std::shared_ptr<RenderingContext> context,
    std::string debug_name)
    : context_(std::move(context)), debug_name_(std::move(debug_name)) {
  is_valid_ = CreateShaderLibrary() &&         //
              CreateDescriptorSetLayout() &&   //
              CreateDescriptorSets() &&        //
              BindDescriptorSets() &&          //
              CreatePipelineLayout() &&        //
              CreateVertexBuffer(vertices) &&  //
              CreateIndexBuffer(indices) &&    //
              CreateUniformBuffer() &&         //
              CreatePipelines();
}

ModelDeviceContext::~ModelDeviceContext() = default;

bool ModelDeviceContext::CreateShaderLibrary() {
  shader_library_ = std::make_unique<ShaderLibrary>(context_->GetDevice());

  shader_library_->AddLiveUpdateCallback([this]() {
    // No need for weak because we own the shader library and it cannot outlast
    // us.
    this->OnShaderLibraryDidUpdate();
  });

  if (!shader_library_->AddDefaultVertexShader(
          "model_renderer.vert",
          MakeStringF("%s Vertex", debug_name_.c_str()).c_str()) ||
      !shader_library_->AddDefaultFragmentShader(
          "model_renderer.frag",
          MakeStringF("%s Fragment", debug_name_.c_str()).c_str())) {
    return false;
  }

  return true;
}

bool ModelDeviceContext::CreateDescriptorSetLayout() {
  descriptor_set_layout_ =
      shaders::model_renderer::UniformBuffer::CreateDescriptorSetLayout(
          context_->GetDevice());

  return static_cast<bool>(descriptor_set_layout_);
}

bool ModelDeviceContext::CreatePipelineLayout() {
  PipelineLayoutBuilder pipeline_layout_builder;
  pipeline_layout_builder.AddDescriptorSetLayout(descriptor_set_layout_.get());

  pipeline_layout_ = pipeline_layout_builder.CreatePipelineLayout(
      context_->GetDevice(), debug_name_.c_str());

  return static_cast<bool>(pipeline_layout_);
}

bool ModelDeviceContext::CreateVertexBuffer(
    const std::vector<pixel::shaders::model_renderer::Vertex>& vertices) {
  vertex_buffer_.reset();

  if (vertices.empty()) {
    return true;
  }

  vertex_buffer_ = context_->GetMemoryAllocator().CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlagBits::eVertexBuffer,                 //
      vertices,                                               //
      context_->GetTransferCommandPool(),                     //
      MakeStringF("%s Vertex", debug_name_.c_str()).c_str(),  //
      nullptr,                                                //
      nullptr,                                                //
      nullptr,                                                //
      nullptr);

  return static_cast<bool>(vertex_buffer_);
}

bool ModelDeviceContext::CreateIndexBuffer(
    const std::vector<uint32_t>& indices) {
  index_buffer_.reset();

  if (indices.empty()) {
    return true;
  }

  index_buffer_ = context_->GetMemoryAllocator().CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlagBits::eIndexBuffer,                 //
      indices,                                               //
      context_->GetTransferCommandPool(),                    //
      MakeStringF("%s Index", debug_name_.c_str()).c_str(),  //
      nullptr,                                               //
      nullptr,                                               //
      nullptr,                                               //
      nullptr);

  return static_cast<bool>(index_buffer_);
}

bool ModelDeviceContext::CreateUniformBuffer() {
  uniform_buffer_ = {
      context_->GetMemoryAllocator(),                          // allocator
      {},                                                      // prototype
      context_->GetSwapchainImageCount(),                      // image count
      MakeStringF("%s Uniform", debug_name_.c_str()).c_str(),  // debug name
  };

  return static_cast<bool>(uniform_buffer_);
}

bool ModelDeviceContext::CreateDescriptorSets() {
  descriptor_sets_.Reset();

  auto descriptor_sets = context_->GetDescriptorPool().AllocateDescriptorSets(
      descriptor_set_layout_.get(),        // layout
      context_->GetSwapchainImageCount(),  // count
      debug_name_.c_str()                  // debug name
  );

  if (!descriptor_sets.IsValid()) {
    return false;
  }

  descriptor_sets_ = std::move(descriptor_sets);
  return true;
}

bool ModelDeviceContext::BindDescriptorSets() {
  auto buffer_infos = uniform_buffer_.GetBufferInfos();
  if (buffer_infos.size() != descriptor_sets_.GetSize()) {
    return false;
  }

  auto write_descriptor_set_generator = [&](size_t index) {
    return std::vector<vk::WriteDescriptorSet>{{
        nullptr,  // dst set (will be filled out later)
        0u,       // binding
        0u,       // array element
        1u,       // descriptor count
        vk::DescriptorType::eUniformBuffer,  // type
        nullptr,                             // image
        &buffer_infos[index],                // buffer
        nullptr,                             // buffer view
    }};
  };

  return descriptor_sets_.UpdateDescriptorSets(write_descriptor_set_generator);
}

bool ModelDeviceContext::CreatePipelines() {
  pipelines_.clear();

  if (required_topologies_.empty()) {
    return true;
  }

  auto vertex_input_bindings =
      shaders::model_renderer::Vertex::GetVertexInputBindings();
  auto vertex_input_attributes =
      shaders::model_renderer::Vertex::GetVertexInputAttributes();
  PipelineBuilder pipeline_builder;
  pipeline_builder.SetDepthStencilTestInfo(
      vk::PipelineDepthStencilStateCreateInfo{
          {},                    // flags
          true,                  // depth test enable
          true,                  // depth write enable
          vk::CompareOp::eLess,  // compare op
          false,                 // depth bounds test enable
          false,                 // stencil test enable
          {},                    // front stencil op state
          {},                    // back stencil op state
          0.0f,                  // min depth bounds
          1.0f                   // max max bounds
      });
  pipeline_builder.AddDynamicState(vk::DynamicState::eViewport);
  pipeline_builder.AddDynamicState(vk::DynamicState::eScissor);
  pipeline_builder.SetVertexInputDescription(vertex_input_bindings,
                                             vertex_input_attributes);
  pipeline_builder.SetFrontFace(vk::FrontFace::eCounterClockwise);

  for (const auto& topology : required_topologies_) {
    pipeline_builder.SetPrimitiveTopology(topology);
    auto pipeline = pipeline_builder.CreatePipeline(
        context_->GetDevice(),                                //
        context_->GetPipelineCache(),                         //
        pipeline_layout_.get(),                               //
        context_->GetOnScreenRenderPass(),                    //
        shader_library_->GetPipelineShaderStageCreateInfos()  //
    );

    if (!pipeline) {
      return false;
    }

    SetDebugNameF(context_->GetDevice(),  //
                  pipeline.get(),         //
                  "%s Pipeline", debug_name_.c_str());

    pipelines_[topology] = std::move(pipeline);
  }
  return true;
}

void ModelDeviceContext::OnShaderLibraryDidUpdate() {
  context_->GetDevice().waitIdle();

  if (!CreatePipelines()) {
    P_ERROR << "Error while rebuilding pipelines.";
  }
}

// *****************************************************************************
// *** ModelDrawData
// *****************************************************************************

ModelDrawData::ModelDrawData() = default;

ModelDrawData::~ModelDrawData() = default;

void ModelDrawData::AddDrawCall(std::shared_ptr<ModelDrawCall> draw_call) {
  if (!draw_call) {
    return;
  }

  draw_calls_.emplace_back(std::move(draw_call));
}

}  // namespace model
}  // namespace pixel
