#include "descriptor_pool.h"

namespace pixel {

DescriptorSets::DescriptorSets() = default;

DescriptorSets::~DescriptorSets() = default;

DescriptorSets::DescriptorSets(DescriptorSets&& other) {
  Reset();
  std::swap(device_, other.device_);
  std::swap(descriptor_sets_, other.descriptor_sets_);
  std::swap(is_valid_, other.is_valid_);
}

DescriptorSets& DescriptorSets::operator=(DescriptorSets&& other) {
  Reset();
  std::swap(device_, other.device_);
  std::swap(descriptor_sets_, other.descriptor_sets_);
  std::swap(is_valid_, other.is_valid_);
  return *this;
}

bool DescriptorSets::IsValid() const {
  return is_valid_;
}

DescriptorSets::operator bool() const {
  return IsValid();
}

void DescriptorSets::Reset() {
  device_ = nullptr;
  descriptor_sets_.clear();
  is_valid_ = false;
}

DescriptorSets::DescriptorSets(
    vk::Device device,
    std::vector<vk::UniqueDescriptorSet> descriptor_sets)
    : device_(device),
      descriptor_sets_(std::move(descriptor_sets)),
      is_valid_(true) {}

bool DescriptorSets::UpdateDescriptorSets(
    std::vector<vk::WriteDescriptorSet> write_descriptor_sets) const {
  if (!is_valid_) {
    return false;
  }

  for (auto& write_set : write_descriptor_sets) {
    for (const auto& desc_set : descriptor_sets_) {
      write_set.setDstSet(desc_set.get());
    }
  }

  device_.updateDescriptorSets(write_descriptor_sets, nullptr);
  return true;
}

DescriptorPool::DescriptorPool(vk::Device device) : device_(device) {
  constexpr size_t kPoolSize = 1024;

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

  vk::DescriptorPoolCreateInfo pool_info = {
      vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,  // flags
      static_cast<uint32_t>(pool_sizes.size() * kPoolSize),  // max sets
      static_cast<uint32_t>(pool_sizes.size()),              // pool sizes count
      pool_sizes.data()                                      // pool sizes
  };

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

DescriptorSets DescriptorPool::AllocateDescriptorSets(
    vk::DescriptorSetLayout layout,
    size_t count) {
  if (!layout || count == 0 || !is_valid_) {
    return {};
  }

  std::vector<vk::DescriptorSetLayout> layouts{count, layout};
  vk::DescriptorSetAllocateInfo descriptor_set_alloc_info = {
      pool_.get(),                   // pool
      static_cast<uint32_t>(count),  // descriptor count
      layouts.data()                 // layouts
  };

  auto result = UnwrapResult(
      device_.allocateDescriptorSetsUnique(descriptor_set_alloc_info));
  if (result.size() != count) {
    return {};
  }
  return {device_, std::move(result)};
}

}  // namespace pixel
