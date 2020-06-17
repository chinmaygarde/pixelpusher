#pragma once

#include <optional>
#include <vector>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class DescriptorPool;

vk::UniqueDescriptorSetLayout CreateDescriptorSetLayoutUnique(
    vk::Device,
    std::vector<vk::DescriptorSetLayoutBinding> bindings);

class DescriptorSets {
 public:
  DescriptorSets();

  DescriptorSets(DescriptorSets&&);

  ~DescriptorSets();

  DescriptorSets& operator=(DescriptorSets&&);

  vk::DescriptorSet operator[](size_t index) const;

  size_t GetSize() const;

  void Reset();

  bool IsValid() const;

  operator bool() const;

  bool UpdateDescriptorSets(
      std::function<std::vector<vk::WriteDescriptorSet>(size_t index)>) const;

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
  DescriptorPool(vk::Device device, std::string debug_name);

  ~DescriptorPool();

  vk::DescriptorPool GetPool() const;

  const vk::DescriptorPool* operator->() const;

  // TODO: This is the wrong abstraction. Rework.
  DescriptorSets AllocateDescriptorSets(vk::DescriptorSetLayout layout,
                                        size_t count,
                                        const char* debug_name);

  std::optional<std::vector<vk::UniqueDescriptorSet>>
  AllocateDescriptorSetsUnique(vk::DescriptorSetLayout layout,
                               size_t count,
                               const char* debug_name);

 private:
  const vk::Device device_;
  const std::string debug_name_;
  vk::UniqueDescriptorPool pool_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(DescriptorPool);
};

}  // namespace pixel
