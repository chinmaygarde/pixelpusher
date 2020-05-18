#pragma once

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class DescriptorPool {
 public:
  DescriptorPool(vk::Device device);

  ~DescriptorPool();

  vk::DescriptorPool GetPool() const;

  const vk::DescriptorPool* operator->() const;

 private:
  vk::UniqueDescriptorPool pool_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(DescriptorPool);
};

}  // namespace pixel
