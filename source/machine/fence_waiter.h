#pragma once

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "closure.h"
#include "macros.h"
#include "vulkan.h"

namespace pixel {

// TODO: There can only be one of these per queue. Enfore this in the API.
class FenceWaiter : public std::enable_shared_from_this<FenceWaiter> {
 public:
  static std::shared_ptr<FenceWaiter> Create(vk::Device device,
                                             vk::Queue queue);

  ~FenceWaiter();

  //----------------------------------------------------------------------------
  /// @brief      Calls the completion handler on the calling thread once the
  ///             fence is signaled on the queue.
  ///
  /// @param[in]  fence    The fence
  /// @param[in]  handler  The handler
  ///
  /// @return     If the completion handler was registered.
  ///
  bool AddCompletionHandler(vk::Fence fence, Closure handler);

  bool IsValid() const;

 private:
  vk::Device device_;
  vk::Queue queue_;
  std::unique_ptr<std::thread> waiter_;
  std::mutex fences_mutex_;
  std::condition_variable fences_cv_;
  std::map<vk::Fence, Closure> awaited_fences_;
  bool is_valid_ = false;

  FenceWaiter(vk::Device device, vk::Queue queue);

  bool StartThread();

  void WaiterMain();

  std::optional<std::vector<vk::Fence>> CreateWaitSetLocked();

  void ProcessSignalledFences(std::vector<vk::Fence> fences);

  P_DISALLOW_COPY_AND_ASSIGN(FenceWaiter);
};

}  // namespace pixel
