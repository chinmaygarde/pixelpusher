#include "filesystem_watcher_win.h"

#include <vector>

#include "logging.h"
#include "win_utils.h"

namespace pixel {

FileSystemWatcherWin::FileSystemWatcherWin()
    : thread_([&]() { WatcherMain(); }) {}

FileSystemWatcherWin::~FileSystemWatcherWin() {
  Terminate();
}

std::optional<size_t> FileSystemWatcherWin::WatchPathForUpdates(
    std::string path,
    Closure change_callback) {
  if (path.empty() || change_callback == nullptr) {
    return std::nullopt;
  }

  auto notification_handle = ::FindFirstChangeNotification(
      path.c_str(),                                            // path
      false,                                                   // watch subtree
      FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE  // change mask
  );

  if (notification_handle == INVALID_HANDLE_VALUE) {
    P_ERROR << "Could not setup the file watcher: " << GetLastErrorMessage();
    return std::nullopt;
  }

  UniqueFirstChange change(notification_handle);

  {
    std::scoped_lock lock(mutex_);
    if (changes_.size() >= MAXIMUM_WAIT_OBJECTS) {
      P_ERROR << "Reached limit of number of objects that can be awaited upon "
                 "on this platform.";
      return std::nullopt;
    }

    changes_.insert(std::move(change));
    callbacks_[last_handle_++] = change_callback;
  }

  callbacks_cv_.notify_one();
  return std::nullopt;
}

bool FileSystemWatcherWin::StopWatchingForUpdates(size_t handle) {
  std::scoped_lock lock(mutex_);

  return false;
}

void FileSystemWatcherWin::WatcherMain() {
  while (!terminated_) {
    std::unique_lock lock(mutex_);

    callbacks_cv_.wait(lock,
                       [&]() { return terminated_ || !changes_.empty(); });

    std::vector<HANDLE> handles;
    for (const auto& change : changes_) {
      handles.push_back(change.Get());
    }

    lock.unlock();

    if (handles.empty()) {
      continue;
    }

    P_ERROR << "Watching " << handles.size() << " handles.";

    // TODO: The new wait of thread termination won't succeed till this timeout
    // expires. Fix this limitation.
    auto status = ::WaitForMultipleObjects(handles.size(),  // count
                                           handles.data(),  // handles
                                           false,           // wait all
                                           250  // timeout milliseconds
    );

    if (status >= WAIT_OBJECT_0 && status < WAIT_OBJECT_0) {
      P_ERROR << "Something changed.";
    }

    switch (status) {
      case WAIT_TIMEOUT:
        continue;
      default:
        P_ERROR << "Error on WaitForMultipleObjects: " << GetLastErrorMessage();
        P_ERROR << "Unhandled error while watching for filesystem changes. "
                   "Stopping all watches. Expect no more notifications.";
        terminated_ = true;
        return;
    }
  }
}

// |FileSystemWatcher|
void FileSystemWatcherWin::Terminate() {
  terminated_ = true;
  callbacks_cv_.notify_one();
}

}  // namespace pixel
