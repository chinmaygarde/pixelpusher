#include "model_draw_data.h"

#include "pipeline_builder.h"
#include "pipeline_layout.h"
#include "string_utils.h"

namespace pixel {
namespace model {

// *****************************************************************************
// *** ModelDrawCall
// *****************************************************************************

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

vk::PrimitiveTopology ModelDrawCall::GetTopology() const {
  return topology_;
}

const std::vector<uint32_t>& ModelDrawCall::GetIndices() const {
  return indices_;
}

const std::vector<pixel::shaders::model_renderer::Vertex>&
ModelDrawCall::GetVertices() const {
  return vertices_;
}

const ModelTextureMap& ModelDrawCall::GetTextures() const {
  return textures_;
}

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
    std::unique_ptr<pixel::Buffer> vertex_buffer,
    std::unique_ptr<pixel::Buffer> index_buffer,
    std::vector<ModelDeviceDrawData> draw_data,
    std::string debug_name)
    : context_(std::move(context)),
      debug_name_(std::move(debug_name)),
      draw_data_(std::move(draw_data)),
      vertex_buffer_(std::move(vertex_buffer)),
      index_buffer_(std::move(index_buffer)) {
  is_valid_ = CreateShaderLibrary() &&        //
              CreateDescriptorSetLayout() &&  //
              CreateDescriptorSets() &&       //
              BindDescriptorSets() &&         //
              CreatePipelineLayout() &&       //
              CreateUniformBuffer() &&        //
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

UniformBuffer<shaders::model_renderer::UniformBuffer>&
ModelDeviceContext::GetUniformBuffer() {
  return uniform_buffer_;
}

bool ModelDeviceContext::Render(vk::CommandBuffer buffer) {
  if (draw_data_.empty()) {
    return true;
  }

  if (!vertex_buffer_) {
    return false;
  }

  if (!uniform_buffer_.UpdateUniformData()) {
    return false;
  }

  const auto uniform_index = uniform_buffer_.GetCurrentIndex();

  if (uniform_index >= descriptor_sets_.GetSize()) {
    return false;
  }

  buffer.setScissor(0u, {context_->GetScissorRect()});
  buffer.setViewport(0u, {context_->GetViewport()});

  for (const auto& draw : draw_data_) {
    auto found = pipelines_.find(draw.topology);
    if (found == pipelines_.end()) {
      return false;
    }

    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, found->second.get());
    buffer.bindVertexBuffers(0u, {vertex_buffer_->buffer},
                             {draw.vertex_buffer_offset});
    buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,  // bind point
        pipeline_layout_.get(),            // layout
        0u,                                // first set
        descriptor_sets_[uniform_index],   // descriptor set
        nullptr                            // dynamic_offsets
    );

    if (draw.index_count > 0u) {
      if (!index_buffer_) {
        return false;
      }

      buffer.bindIndexBuffer(index_buffer_->buffer, draw.index_buffer_offset,
                             vk::IndexType::eUint32);
      buffer.drawIndexed(draw.index_count,  // index count
                         1u,                // instance count
                         0u,                // first index
                         0u,                // vertex offset
                         0u                 // first instance
      );
    } else {
      buffer.draw(draw.vertex_count,  // vertex count
                  1u,                 // instance count
                  0u,                 // first vertex
                  0u                  // first instance
      );
    }
  }

  return false;
}

// *****************************************************************************
// *** ModelDrawData
// *****************************************************************************

ModelDrawData::ModelDrawData(std::string debug_name)
    : debug_name_(std::move(debug_name)) {}

ModelDrawData::~ModelDrawData() = default;

bool ModelDrawData::AddDrawCall(std::shared_ptr<ModelDrawCall> draw_call) {
  if (!draw_call) {
    return false;
  }

  draw_calls_.emplace_back(std::move(draw_call));
  return true;
}

std::unique_ptr<pixel::Buffer> ModelDrawData::CreateVertexBuffer(
    const RenderingContext& context) const {
  vk::DeviceSize vertex_buffer_size = 0;
  for (const auto& call : draw_calls_) {
    const auto& vertices = call->GetVertices();
    vertex_buffer_size +=
        vertices.size() * sizeof(ModelDrawCall::VertexValueType);
  }

  auto copy_callback = [&](uint8_t* staging_buffer,
                           size_t staging_buffer_size) -> bool {
    if (staging_buffer_size < vertex_buffer_size) {
      return false;
    }

    size_t current_size = 0;
    for (const auto& call : draw_calls_) {
      const auto& vertices = call->GetVertices();
      const size_t copy_size =
          vertices.size() * sizeof(ModelDrawCall::VertexValueType);
      ::memcpy(staging_buffer + staging_buffer_size,  //
               vertices.data(),                       //
               copy_size                              //
      );
      current_size += copy_size;
    }

    return true;
  };

  return context.GetMemoryAllocator().CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlagBits::eVertexBuffer,                   //
      copy_callback,                                            //
      vertex_buffer_size,                                       //
      context.GetTransferCommandPool(),                         //
      MakeStringF("%s Vertices", debug_name_.c_str()).c_str(),  //
      nullptr,                                                  //
      nullptr,                                                  //
      nullptr,                                                  //
      nullptr                                                   //
  );
}

std::unique_ptr<pixel::Buffer> ModelDrawData::CreateIndexBuffer(
    const RenderingContext& context) const {
  vk::DeviceSize index_buffer_size = 0;
  for (const auto& call : draw_calls_) {
    const auto& indices = call->GetIndices();
    index_buffer_size += indices.size() * sizeof(ModelDrawCall::IndexValueType);
  }

  auto copy_callback = [&](uint8_t* staging_buffer,
                           size_t staging_buffer_size) -> bool {
    if (staging_buffer_size < index_buffer_size) {
      return false;
    }

    size_t current_size = 0;
    for (const auto& call : draw_calls_) {
      const auto& indices = call->GetIndices();
      const size_t copy_size =
          indices.size() * sizeof(ModelDrawCall::IndexValueType);
      ::memcpy(staging_buffer + staging_buffer_size,  //
               indices.data(),                        //
               copy_size                              //
      );
      current_size += copy_size;
    }

    return true;
  };

  return context.GetMemoryAllocator().CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlagBits::eVertexBuffer,                  //
      copy_callback,                                           //
      index_buffer_size,                                       //
      context.GetTransferCommandPool(),                        //
      MakeStringF("%s Indices", debug_name_.c_str()).c_str(),  //
      nullptr,                                                 //
      nullptr,                                                 //
      nullptr,                                                 //
      nullptr                                                  //
  );
}

std::unique_ptr<ModelDeviceContext> ModelDrawData::CreateModelDeviceContext(
    std::shared_ptr<RenderingContext> context) const {
  if (!context || context->IsValid()) {
    return nullptr;
  }

  auto vertex_buffer = CreateVertexBuffer(*context);
  auto index_buffer = CreateIndexBuffer(*context);

  // TODO: Remove this and add fence waits.
  context->GetDevice().waitIdle();

  std::set<vk::PrimitiveTopology> required_topologies;
  std::vector<ModelDeviceDrawData> draw_data;
  vk::DeviceSize vertex_buffer_offset = 0;
  vk::DeviceSize index_buffer_offset = 0;

  for (const auto& call : draw_calls_) {
    required_topologies.insert(call->GetTopology());

    ModelDeviceDrawData data;
    data.topology = call->GetTopology();
    data.vertex_buffer_offset = vertex_buffer_offset;
    data.index_buffer_offset = index_buffer_offset;
    data.vertex_count = call->GetVertices().size();
    data.index_count = call->GetIndices().size();

    vertex_buffer_offset +=
        call->GetVertices().size() * sizeof(ModelDrawCall::VertexValueType);
    index_buffer_offset +=
        call->GetIndices().size() * sizeof(ModelDrawCall::IndexValueType);

    draw_data.push_back(data);
  }

  return std::make_unique<ModelDeviceContext>(context,                   //
                                              std::move(vertex_buffer),  //
                                              std::move(index_buffer),   //
                                              std::move(draw_data),      //
                                              debug_name_                //
  );
}

bool ModelDrawData::RegisterSampler(std::shared_ptr<Sampler> sampler) {
  return false;
}

bool ModelDrawData::RegisterImage(std::shared_ptr<Image> image) {
  return false;
}

}  // namespace model
}  // namespace pixel
