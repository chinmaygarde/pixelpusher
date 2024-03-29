#include "tutorial_renderer.h"

#include <imgui.h>

#include "assets_location.h"
#include "command_buffer.h"
#include "file.h"
#include "glm.h"
#include "logging.h"
#include "pipeline_builder.h"
#include "pipeline_layout.h"
#include "shader_module.h"
#include "vulkan.h"

namespace pixel {

TutorialRenderer::TutorialRenderer(std::shared_ptr<RenderingContext> context)
    : Renderer(std::move(context)),
      device_(GetContext().GetDevice()),
      shader_library_(device_) {
  is_valid_ = true;
  shader_library_.AddLiveUpdateCallback([this]() {
    P_LOG << "Tutorial shader updated. Rebuilding pipelines.";
    if (!RebuildPipelines()) {
      P_ERROR << "Could not rebuild pipelines.";
    }
  });
}

TutorialRenderer::~TutorialRenderer() = default;

bool TutorialRenderer::IsValid() const {
  return is_valid_;
}

bool TutorialRenderer::Setup() {
  if (!is_valid_) {
    return false;
  }

  if (!shader_library_.AddDefaultVertexShader("triangle.vert",
                                              "Triangle Vertex") ||
      !shader_library_.AddDefaultFragmentShader("triangle.frag",
                                                "Triangle Fragment")) {
    P_ERROR << "Could not build shader library.";
    return false;
  }

  const std::vector<TriangleVertices> vertices = {
      {{-0.5f, -0.5f, 0.1f}, {0.0f, 0.0f}},  // 0
      {{0.5f, -0.5f, 0.1f}, {1.0f, 0.0f}},   // 1
      {{0.5f, 0.5f, 0.1f}, {1.0f, 1.0f}},    // 2
      {{-0.5f, 0.5f, 0.1f}, {0.0f, 1.0f}},   // 3

      {{-0.5f, -0.5f, 0.2f}, {0.0f, 0.0f}},  // 4
      {{0.5f, -0.5f, 0.2f}, {1.0f, 0.0f}},   // 5
      {{0.5f, 0.5f, 0.2f}, {1.0f, 1.0f}},    // 6
      {{-0.5f, 0.5f, 0.2f}, {0.0f, 1.0f}},   // 7
  };

  const std::vector<uint16_t> indices = {
      4, 7, 6, 6, 5, 4,  // bottom
      2, 1, 0, 0, 3, 2,  // top
  };

  auto vertex_buffer =
      GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
          vk::BufferUsageFlagBits::eVertexBuffer,                    // usage
          vertices.data(),                                           // data
          vertices.size() * sizeof(decltype(vertices)::value_type),  // size
          GetContext().GetTransferCommandPool(),                     // pool
          "Tutorial Vertex",  // debug name
          nullptr,            // wait semaphores
          nullptr,            // wait stages
          nullptr,            // signal semaphores
          nullptr             // on done
      );

  auto index_buffer =
      GetContext().GetMemoryAllocator().CreateDeviceLocalBufferCopy(
          vk::BufferUsageFlagBits::eIndexBuffer,                   // usage
          indices.data(),                                          // data
          indices.size() * sizeof(decltype(indices)::value_type),  // size
          GetContext().GetTransferCommandPool(),                   // pool
          "Tutorial Index",                                        // debug name
          nullptr,  // wait semaphores
          nullptr,  // wait stages
          nullptr,  // signal semaphores
          nullptr   // on done
      );

  auto image_file = OpenFile(PIXEL_ASSETS_LOCATION "Nighthawks.jpg");
  if (!image_file) {
    return false;
  }

  ImageDecoder decoder(*image_file);
  if (!decoder.IsValid()) {
    return false;
  }

  auto image = decoder.CreateDeviceLocalImageCopy(
      GetContext().GetMemoryAllocator(),      // allocator
      GetContext().GetTransferCommandPool(),  // command pool
      "Nighthawks",                           // debug name
      nullptr,                                // wait semaphores
      nullptr,                                // wait staged
      nullptr,                                // signal semaphores
      nullptr                                 // on done
  );

  if (!vertex_buffer || !index_buffer || !image) {
    P_ERROR << "Could not allocate either the vertex buffer, index buffer, or "
               "image";
    return false;
  }

  // TODO: For the transfer to the staging buffer to be complete. Get rid of
  // this and use fences instead.
  device_.waitIdle();

  vertex_buffer_ = std::move(vertex_buffer);
  index_buffer_ = std::move(index_buffer);
  image_ = std::move(image);

  UniformBuffer<TriangleUBO> triangle_ubo(GetContext().GetMemoryAllocator(), {},
                                          GetContext().GetSwapchainImageCount(),
                                          "Tutorial Uniform");

  if (!triangle_ubo) {
    P_ERROR << "Could not allocate backing store for the triangle UBO.";
    return false;
  }

  triangle_ubo_ = std::move(triangle_ubo);

  auto descriptor_set_layout = TriangleUBO::CreateDescriptorSetLayout(device_);

  if (!descriptor_set_layout) {
    P_ERROR << "Could not create descriptor set layout.";
    return false;
  }

  descriptor_set_layout_ = std::move(descriptor_set_layout);

  vk::DescriptorSetAllocateInfo descriptor_info;
  descriptor_info.setDescriptorPool(GetContext().GetDescriptorPool().GetPool());
  descriptor_info.setDescriptorSetCount(GetContext().GetSwapchainImageCount());
  auto layouts = std::vector<vk::DescriptorSetLayout>{
      GetContext().GetSwapchainImageCount(), descriptor_set_layout_.get()};
  descriptor_info.setPSetLayouts(layouts.data());

  descriptor_sets_ =
      UnwrapResult(device_.allocateDescriptorSets(descriptor_info));

  if (descriptor_sets_.size() != GetContext().GetSwapchainImageCount()) {
    P_ERROR << "Could not allocate descriptor sets.";
    return false;
  }

  vk::SamplerCreateInfo sampler_info;
  sampler_info.setMagFilter(vk::Filter::eLinear);
  sampler_info.setMinFilter(vk::Filter::eLinear);
  sampler_info.setMipmapMode(vk::SamplerMipmapMode::eNearest);
  sampler_info.setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
  sampler_info.setAddressModeV(vk::SamplerAddressMode::eClampToEdge);
  sampler_info.setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
  sampler_info.setMipLodBias(0.0f);

  if (GetContext().GetFeatures().samplerAnisotropy) {
    sampler_info.setAnisotropyEnable(true);
    sampler_info.setMaxAnisotropy(16.0f);
  }

  sampler_info.setCompareEnable(false);
  sampler_info.setCompareOp(vk::CompareOp::eAlways);
  sampler_info.setUnnormalizedCoordinates(false);
  sampler_info.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
  sampler_info.setMinLod(0.0f);
  sampler_info.setMaxLod(0.0f);

  sampler_ = UnwrapResult(device_.createSamplerUnique(sampler_info));

  if (!sampler_) {
    P_ERROR << "Could not create sampler.";
    return false;
  }

  std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
  auto triangle_ubo_buffer_infos = triangle_ubo_.GetBufferInfos();
  vk::DescriptorImageInfo triangle_sampler_info;
  triangle_sampler_info.setSampler(sampler_.get());
  triangle_sampler_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
  triangle_sampler_info.setImageView(image_->GetImageView());

  for (size_t i = 0; i < descriptor_sets_.size(); i++) {
    {
      vk::WriteDescriptorSet write_descriptor_set;
      write_descriptor_set.setDstSet(descriptor_sets_[i]);
      write_descriptor_set.setDstBinding(0u);
      write_descriptor_set.setDstArrayElement(0u);
      write_descriptor_set.setDescriptorCount(1u);  // one UBO
      write_descriptor_set.setDescriptorType(
          vk::DescriptorType::eUniformBuffer);

      write_descriptor_set.setPBufferInfo(&triangle_ubo_buffer_infos[i]);

      write_descriptor_sets.push_back(write_descriptor_set);
    }

    {
      vk::WriteDescriptorSet write_descriptor_set;
      write_descriptor_set.setDstSet(descriptor_sets_[i]);
      write_descriptor_set.setDstBinding(1u);
      write_descriptor_set.setDstArrayElement(0u);
      write_descriptor_set.setDescriptorCount(1u);  // one CombinedImageSampler
      write_descriptor_set.setDescriptorType(
          vk::DescriptorType::eCombinedImageSampler);

      write_descriptor_set.setPImageInfo(&triangle_sampler_info);

      write_descriptor_sets.push_back(write_descriptor_set);
    }
  }

  device_.updateDescriptorSets(write_descriptor_sets, nullptr);

  std::vector<vk::DescriptorSetLayout> layouts2;
  layouts2.push_back(descriptor_set_layout_.get());

  // Setup pipeline layout.
  pipeline_layout_ = CreatePipelineLayout(device_, layouts2);

  if (!RebuildPipelines()) {
    P_ERROR << "Could not build pipelines.";
    return false;
  }

  return true;
}  // namespace pixel

// |Renderer|
bool TutorialRenderer::BeginFrame() {
  // Nothing to do here.
  return true;
}

// |Renderer|s
bool TutorialRenderer::RenderFrame(vk::CommandBuffer command_buffer) {
  if (!is_valid_) {
    P_ERROR << "Render was not valid.";
    return false;
  }

  const auto extents = GetContext().GetExtents();

  ::ImGui::Begin("Tutorial");

  ::ImGui::SliderFloat("FOV", &fov_, 1.0f, 180.0f);
  ::ImGui::InputFloat("Rotation Rate", &rotation_rate_);
  ::ImGui::SliderFloat("zNear", &z_near_, 0.001f, 10.0f);
  ::ImGui::SliderFloat("zFar", &z_far_, 0.001f, 10.0f);
  ::ImGui::SliderFloat3("Eye", eye_, 0.0f, 2.0f);

  ::ImGui::End();

  auto rate =
      std::chrono::duration<float>(Clock::now().time_since_epoch()).count();
  triangle_ubo_->model =
      glm::rotate(glm::mat4(1.0),                              // model
                  glm::radians<float>(rate * rotation_rate_),  // radians
                  glm::vec3(0.0f, 0.0f, 1.0f)                  // center
      );
  triangle_ubo_->view =
      glm::lookAt(glm::vec3(eye_[0], eye_[1], eye_[2]),  // eye
                  glm::vec3(0.0f, 0.0f, 0.0f),           // center
                  glm::vec3(0.0f, 0.0f, 1.0f)            // up
      );
  triangle_ubo_->projection = glm::perspective(
      glm::radians(fov_),
      static_cast<float>(extents.width) / static_cast<float>(extents.height),
      z_near_, z_far_);

  if (!triangle_ubo_.UpdateUniformData()) {
    P_ERROR << "Could not write uniform data.";
    return false;
  }

  auto marker = DebugMarkerBegin(command_buffer, "Draw Triangle");
  // Perform per frame rendering operations here.
  command_buffer.setScissor(0u, GetContext().GetScissorRect());
  command_buffer.setViewport(0u, GetContext().GetViewport());
  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                              pipeline_.get());
  command_buffer.bindVertexBuffers(0u, {vertex_buffer_->buffer}, {0u});
  command_buffer.bindIndexBuffer(index_buffer_->buffer, 0,
                                 vk::IndexType::eUint16);
  command_buffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, pipeline_layout_.get(), 0u,
      descriptor_sets_[triangle_ubo_.GetCurrentIndex()], nullptr);
  command_buffer.drawIndexed(12, 1, 0, 0, 0);

  return true;
}

bool TutorialRenderer::Teardown() {
  return true;
}

bool TutorialRenderer::RebuildPipelines() {
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

  pipeline_builder.SetVertexInputDescription(
      {TriangleVertices::GetVertexInputBindingDescription()},
      TriangleVertices::GetVertexInputAttributeDescription());
  pipeline_builder.AddDynamicState(vk::DynamicState::eScissor);
  pipeline_builder.AddDynamicState(vk::DynamicState::eViewport);

  const auto& shader_stages =
      shader_library_.GetPipelineShaderStageCreateInfos();

  auto pipeline = pipeline_builder.CreatePipeline(
      device_,                               // device
      GetContext().GetPipelineCache(),       // pipeline cache
      pipeline_layout_.get(),                // pipeline layout
      GetContext().GetOnScreenRenderPass(),  // render pass
      shader_stages                          // shader stages
  );

  if (!pipeline) {
    P_ERROR << "Could not create pipeline.";
    return false;
  }

  SetDebugName(device_, pipeline.get(), "Triangle Pipeline");

  pipeline_ = std::move(pipeline);

  return true;
}

}  // namespace pixel
