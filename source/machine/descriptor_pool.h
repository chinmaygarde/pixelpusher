#pragma once

#include <optional>
#include <vector>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class DescriptorPool;

class DescriptorSets {
 public:
  DescriptorSets();

  DescriptorSets(DescriptorSets&&);

  ~DescriptorSets();

  DescriptorSets& operator=(DescriptorSets&&);

  void Reset();

  bool IsValid() const;

  operator bool() const;

  bool UpdateDescriptorSets(
      std::vector<vk::WriteDescriptorSet> write_descriptor_sets) const;

 private:
  friend class DescriptorPool;

  vk::Device device_;
  std::vector<vk::UniqueDescriptorSet> descriptor_sets_;
  bool is_valid_ = false;

  DescriptorSets(vk::Device device,
                 std::vector<vk::UniqueDescriptorSet> descriptor_sets);

  P_DISALLOW_COPY_AND_ASSIGN(DescriptorSets);
};

class DescriptorPool {
 public:
  DescriptorPool(vk::Device device);

  ~DescriptorPool();

  vk::DescriptorPool GetPool() const;

  const vk::DescriptorPool* operator->() const;

  DescriptorSets AllocateDescriptorSets(vk::DescriptorSetLayout layout,
                                        size_t count);

 private:
  const vk::Device device_;
  vk::UniqueDescriptorPool pool_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(DescriptorPool);
};

}  // namespace pixel
