#pragma once

#include <Windows.h>

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <set>

#include "closure.h"
#include "filesystem_watcher.h"
#include "logging.h"
#include "macros.h"
#include "thread.h"
#include "unique_object.h"

namespace pixel {

struct FirstChangeObjectTraits {
  static bool IsValid(HANDLE fd) { return INVALID_HANDLE_VALUE == fd; }

  static HANDLE DefaultValue() { return INVALID_HANDLE_VALUE; }

  static void Collect(HANDLE fd) {
    if (!::FindCloseChangeNotification(fd)) {
      P_ERROR << "Could not close first change handle.";
    }
  }
};

using UniqueFirstChange = UniqueObject<HANDLE, FirstChangeObjectTraits>;

class FileSystemWatcherWin final : public FileSystemWatcher {
 public:
  FileSystemWatcherWin();

  // |FileSystemWatcher|
  ~FileSystemWatcherWin() override;

  // |FileSystemWatcher|
  std::optional<size_t> WatchPathForUpdates(std::string path,
                                            Closure change_callback) override;

  // |FileSystemWatcher|
  bool StopWatchingForUpdates(size_t handle) override;

  // |FileSystemWatcher|
  void Terminate() override;

 private:
  std::mutex mutex_;
  size_t last_handle_ = 1;
  std::map<size_t, Closure> callbacks_;
  std::set<UniqueFirstChange> changes_;
  std::thread thread_;
  std::condition_variable callbacks_cv_;
  std::atomic_bool terminated_ = false;

  void WatcherMain();

  P_DISALLOW_COPY_AND_ASSIGN(FileSystemWatcherWin);
};

}  // namespace pixel
