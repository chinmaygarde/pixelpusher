#pragma once

#include "hash.h"
#include "vulkan.h"

namespace pixel {

struct QueueSelection {
  uint32_t queue_family_index = 0;
  vk::Queue queue;

  struct Hash {
    constexpr std::size_t operator()(const QueueSelection& q) const {
      return HashCombine(q.queue_family_index, q.queue);
    }
  };

  struct Equal {
    constexpr bool operator()(const QueueSelection& lhs,
                              const QueueSelection& rhs) const {
      return (lhs.queue_family_index == rhs.queue_family_index) &&
             (lhs.queue == rhs.queue);
    }
  };
};

}  // namespace pixel
