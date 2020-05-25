#include "model_renderer.h"

#include <future>

#include "pipeline_layout.h"

namespace pixel {

ModelRenderer::ModelRenderer(std::shared_ptr<RenderingContext> context,
                             std::string model_assets_dir,
                             std::string model_path)
    : Renderer(context) {
  std::promise<std::unique_ptr<model::Model>> model_promise;
  auto model_future = model_promise.get_future();
  AssetLoader::GetGlobal()->LoadAsset(
      model_assets_dir, model_path,
      MakeCopyable([promise = std::move(model_promise)](
                       std::unique_ptr<Asset> asset) mutable {
        if (!asset) {
          promise.set_value(nullptr);
          return;
        }
        promise.set_value(std::make_unique<model::Model>(*asset));
      }));
  model_ = model_future.get();
  is_valid_ = true;
}

ModelRenderer::~ModelRenderer() = default;

// |Renderer|
bool ModelRenderer::IsValid() const {
  return is_valid_;
}

// |Renderer|
bool ModelRenderer::Setup() {
  if (!is_valid_) {
    return false;
  }

  ShaderLibrary library(GetContext().GetDevice());

  if (!library.AddDefaultVertexShader("model_renderer.vert") ||
      !library.AddDefaultFragmentShader("model_renderer.frag")) {
    return false;
  }

  descriptor_set_layout_ =
      shaders::model_renderer::UniformBuffer::CreateDescriptorSetLayout(
          GetContext().GetDevice());

  if (!descriptor_set_layout_) {
    return false;
  }

  descriptor_sets_ = GetContext().GetDescriptorPool().AllocateDescriptorSets(
      descriptor_set_layout_.get(), GetContext().GetSwapchainImageCount());

  if (!descriptor_sets_) {
    return false;
  }

  PipelineLayoutBuilder pipeline_layout_builder;
  pipeline_layout_builder.AddDescriptorSetLayout(descriptor_set_layout_.get());

  pipeline_layout_ =
      pipeline_layout_builder.CreatePipelineLayout(GetContext().GetDevice());

  if (!pipeline_layout_) {
    return false;
  }

  auto vertex_input_bindings =
      shaders::model_renderer::Vertex::GetVertexInputBindings();
  auto vertex_input_attributes =
      shaders::model_renderer::Vertex::GetVertexInputAttributes();

  PipelineBuilder pipeline_builder;
  pipeline_builder.AddDynamicState(vk::DynamicState::eViewport);
  pipeline_builder.AddDynamicState(vk::DynamicState::eScissor);
  pipeline_builder.SetVertexInputDescription(vertex_input_bindings,
                                             vertex_input_attributes);

  pipeline_ = pipeline_builder.CreatePipeline(
      GetContext().GetDevice(),                    //
      GetContext().GetPipelineCache(),             //
      pipeline_layout_.get(),                      //
      GetContext().GetOnScreenRenderPass(),        //
      library.GetPipelineShaderStageCreateInfos()  //
  );

  uniform_buffer_ = {
      GetContext().GetMemoryAllocator(),     // allocator
      {},                                    // prototype
      GetContext().GetSwapchainImageCount()  // image count
  };

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

  if (!descriptor_sets_.UpdateDescriptorSets(write_descriptor_set_generator)) {
    return false;
  }

  std::vector<shaders::model_renderer::Vertex> vertices = {
      {{-0.5, 0.5, 0.0}},
      {{0.5, -0.5, 0.0}},
      {{-0.5, -0.5, 0.0}},
  };

  std::vector<uint32_t> indices = {0, 1, 2};

  vertex_buffer_ =
      GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
          vk::BufferUsageFlagBits::eVertexBuffer, vertices,
          GetContext().GetTransferCommandPool(), nullptr, nullptr, nullptr,
          nullptr);
  index_buffer_ = GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
      vk::BufferUsageFlagBits::eIndexBuffer, indices,
      GetContext().GetTransferCommandPool(), nullptr, nullptr, nullptr,
      nullptr);

  GetContext().GetTransferQueue().queue.waitIdle();

  if (!vertex_buffer_ || !index_buffer_) {
    return false;
  }

  index_count_ = indices.size();

  return true;
}

// |Renderer|
bool ModelRenderer::Render(vk::CommandBuffer buffer) {
  if (index_count_ == 0) {
    return true;
  }

  const auto extents = GetContext().GetExtents();

  // auto view =
  //     glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
  //                 glm::vec3(0.0f, 0.0f, 1.0f));
  auto projection =
      glm::ortho(0.0f, static_cast<float>(extents.width),
                 static_cast<float>(extents.height), 0.0f, 0.0f, 1.0f);

  uniform_buffer_->mvp = projection;

  if (!uniform_buffer_.UpdateUniformData()) {
    return false;
  }

  const auto uniform_index = uniform_buffer_.GetCurrentIndex();

  buffer.setScissor(0u, {GetContext().GetScissorRect()});
  buffer.setViewport(0u, {GetContext().GetViewport()});
  buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.get());
  buffer.bindVertexBuffers(0u, {vertex_buffer_->buffer}, {0u});
  buffer.bindIndexBuffer(index_buffer_->buffer, 0u, vk::IndexType::eUint32);
  buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,  // bind point
                            pipeline_layout_.get(),            // layout
                            0u,                                // first set
                            descriptor_sets_[uniform_index],   // descriptor set
                            nullptr  // dynamic_offsets
  );
  buffer.drawIndexed(index_count_,  // index count
                     1u,            // instance count
                     0u,            // first index
                     0u,            // vertex offset
                     0u             // first instance
  );
  return true;
}

// |Renderer|
bool ModelRenderer::Teardown() {
  return true;
}

}  // namespace pixel
