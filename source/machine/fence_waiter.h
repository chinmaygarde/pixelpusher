#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "macros.h"
#include "vulkan.h"

namespace pixel {

class FenceWaiter : public std::enable_shared_from_this<FenceWaiter> {
 public:
  static std::shared_ptr<FenceWaiter> Create(vk::Device device,
                                             vk::Queue queue);

  ~FenceWaiter();

  bool AddCompletionHandler(vk::Fence fence, std::function<void(void)> handler);

  bool IsValid() const;

 private:
  vk::Device device_;
  vk::Queue queue_;
  std::unique_ptr<std::thread> waiter_;
  std::mutex fences_mutex_;
  vk::UniqueFence host_fence_;
  std::map<vk::Fence, std::function<void(void)>> awaited_fences_;
  bool is_valid_ = false;

  FenceWaiter(vk::Device device, vk::Queue queue);

  void WaiterMain();

  std::optional<std::vector<vk::Fence>> CreateWaitSet();

  void ProcessSignalledFences(std::vector<vk::Fence> fences);

  P_DISALLOW_COPY_AND_ASSIGN(FenceWaiter);
};

}  // namespace pixel
