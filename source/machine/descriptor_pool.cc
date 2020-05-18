#include "descriptor_pool.h"

namespace pixel {

DescriptorPool::DescriptorPool(vk::Device device) {
  constexpr size_t kPoolSize = 1024;
  vk::DescriptorPoolCreateInfo pool_info;
  std::vector<vk::DescriptorPoolSize> pool_sizes = {
      {vk::DescriptorType::eSampler, kPoolSize},
      {vk::DescriptorType::eCombinedImageSampler, kPoolSize},
      {vk::DescriptorType::eSampledImage, kPoolSize},
      {vk::DescriptorType::eStorageImage, kPoolSize},
      {vk::DescriptorType::eUniformTexelBuffer, kPoolSize},
      {vk::DescriptorType::eStorageTexelBuffer, kPoolSize},
      {vk::DescriptorType::eUniformBuffer, kPoolSize},
      {vk::DescriptorType::eStorageBuffer, kPoolSize},
      {vk::DescriptorType::eUniformBufferDynamic, kPoolSize},
      {vk::DescriptorType::eStorageBufferDynamic, kPoolSize},
      {vk::DescriptorType::eInputAttachment, kPoolSize},
  };
  pool_info.setMaxSets(pool_sizes.size() * kPoolSize);
  pool_info.setPoolSizeCount(pool_sizes.size());
  pool_info.setPPoolSizes(pool_sizes.data());

  pool_ = UnwrapResult(device.createDescriptorPoolUnique(pool_info));

  if (!pool_) {
    return;
  }

  is_valid_ = true;
}

DescriptorPool::~DescriptorPool() = default;

vk::DescriptorPool DescriptorPool::GetPool() const {
  return *pool_;
}

const vk::DescriptorPool* DescriptorPool::operator->() const {
  return &pool_.get();
}

}  // namespace pixel
