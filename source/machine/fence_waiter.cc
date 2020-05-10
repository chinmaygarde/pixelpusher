#include "fence_waiter.h"

#include <algorithm>
#include <limits>

#include "logging.h"

namespace pixel {

FenceWaiter::FenceWaiter(vk::Device device, vk::Queue queue)
    : device_(std::move(device)), queue_(std::move(queue)) {
  is_valid_ = true;
}

FenceWaiter::~FenceWaiter() = default;

bool FenceWaiter::StartThread() {
  if (waiter_) {
    P_ERROR << "Waiter thread can only be started once.";
    return false;
  }
  waiter_ = std::make_unique<std::thread>(
      [thiz = shared_from_this()]() { thiz->WaiterMain(); });
  return true;
}

std::shared_ptr<FenceWaiter> FenceWaiter::Create(vk::Device device,
                                                 vk::Queue queue) {
  auto waiter = std::shared_ptr<FenceWaiter>(
      new FenceWaiter(std::move(device), std::move(queue)));
  if (!waiter->IsValid()) {
    P_ERROR << "Could not create fence waiter.";
    return nullptr;
  }

  if (!waiter->StartThread()) {
    P_ERROR << "Could not start waiter thread.";
    return nullptr;
  }

  return waiter;
}

bool FenceWaiter::IsValid() const {
  return is_valid_;
}

void FenceWaiter::WaiterMain() {
  while (true) {
    std::unique_lock lock(fences_mutex_);

    fences_cv_.wait(lock, [&]() { return awaited_fences_.size() > 0u; });

    auto wait_set = CreateWaitSetLocked();

    lock.unlock();

    if (!wait_set.has_value()) {
      P_ERROR << "Could not create wait set.";
      return;
    }

    // This should be caught by the predicate.
    P_ASSERT(wait_set.value().size() > 0u);

    auto wait_result =
        device_.waitForFences(wait_set.value(),  // wait set
                              false,             // wait for all
                              std::numeric_limits<uint64_t>::max()  // timeout
        );

    if (wait_result != vk::Result::eSuccess) {
      P_ERROR << "Could not wait on fence wait set.";
      return;
    }

    // If fences become signalled after they are checked here, the next wait
    // will handle them.
    std::vector<vk::Fence> signalled_fences;
    for (const auto& fence : wait_set.value()) {
      const auto fence_is_signalled =
          device_.getFenceStatus(fence) == vk::Result::eSuccess;
      if (fence_is_signalled) {
        signalled_fences.push_back(fence);
      }
    }

    ProcessSignalledFences(std::move(signalled_fences));
  }
}

std::optional<std::vector<vk::Fence>> FenceWaiter::CreateWaitSetLocked() {
  std::vector<vk::Fence> wait_set;
  wait_set.reserve(awaited_fences_.size());
  for (const auto& fence : awaited_fences_) {
    wait_set.push_back(fence.first);
  }
  return wait_set;
}

void FenceWaiter::ProcessSignalledFences(std::vector<vk::Fence> fences) {
  std::vector<std::function<void(void)>> callbacks_to_fire;

  {
    // All access to fences must be guarded by the mutex.
    std::scoped_lock lock(fences_mutex_);
    for (const auto& fence : fences) {
      auto found = awaited_fences_.find(fence);
      // A fence we weren't waiting on cannot be signalled.
      P_ASSERT(found != awaited_fences_.end());
      callbacks_to_fire.push_back(found->second);
      awaited_fences_.erase(found);
    }
  }

  // Fire the callbacks after the fences mutex has been released.
  for (const auto& callback : callbacks_to_fire) {
    callback();
  }
}

bool FenceWaiter::AddCompletionHandler(vk::Fence fence,
                                       std::function<void(void)> handler) {
  if (!fence || !handler) {
    return false;
  }

  std::scoped_lock lock(fences_mutex_);
  awaited_fences_[fence] = handler;
  fences_cv_.notify_one();
  return true;
}

}  // namespace pixel
