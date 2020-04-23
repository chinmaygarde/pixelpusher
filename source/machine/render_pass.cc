#include "render_pass.h"

#include "logging.h"

namespace pixel {

vk::UniqueRenderPass CreateRenderPass(const vk::Device& device,
                                      vk::Format color_attachment_format) {
  vk::AttachmentDescription color_attachment_desc;
  color_attachment_desc.setFormat(color_attachment_format);
  color_attachment_desc.setSamples(vk::SampleCountFlagBits::e1);
  color_attachment_desc.setLoadOp(vk::AttachmentLoadOp::eClear);
  color_attachment_desc.setStoreOp(vk::AttachmentStoreOp::eStore);
  color_attachment_desc.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
  color_attachment_desc.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  color_attachment_desc.setInitialLayout(vk::ImageLayout::eUndefined);
  color_attachment_desc.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference color_attachment_reference;
  color_attachment_reference.setAttachment(0);
  color_attachment_reference.setLayout(
      vk::ImageLayout::eColorAttachmentOptimal);

  vk::SubpassDescription subpass;
  subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
  subpass.setColorAttachmentCount(1u);
  subpass.setPColorAttachments(&color_attachment_reference);

  vk::RenderPassCreateInfo render_pass_info;
  render_pass_info.setAttachmentCount(1u);
  render_pass_info.setPAttachments(&color_attachment_desc);
  render_pass_info.setSubpassCount(1u);
  render_pass_info.setPSubpasses(&subpass);

  auto render_pass_result = device.createRenderPassUnique(render_pass_info);

  if (render_pass_result.result != vk::Result::eSuccess) {
    P_ERROR << "Could not create render pass";
    return {};
  }

  return std::move(render_pass_result.value);
}

}  // namespace pixel
