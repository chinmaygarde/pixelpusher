#pragma once

#include <Windows.h>

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <set>

#include "closure.h"
#include "file.h"
#include "filesystem_watcher.h"
#include "logging.h"
#include "macros.h"
#include "thread.h"
#include "unique_object.h"

namespace pixel {

class PendingFileSystemWatch;

class FileSystemWatcherWin final : public FileSystemWatcher {
 public:
  FileSystemWatcherWin();

  // |FileSystemWatcher|
  ~FileSystemWatcherWin() override;

  // |FileSystemWatcher|
  std::optional<size_t> WatchPathForUpdates(std::filesystem::path path,
                                            Closure change_callback) override;

  // |FileSystemWatcher|
  bool StopWatchingForUpdates(size_t handle) override;

  // |FileSystemWatcher|
  void Terminate() override;

 private:
  UniqueFD completion_port_;
  std::thread thread_;
  std::mutex mutex_;
  std::atomic_size_t last_handle_ = 0;
  std::map<size_t, std::unique_ptr<PendingFileSystemWatch>> watches_;
  std::atomic_bool terminated_ = false;

  void WatcherMain();

  PendingFileSystemWatch* GetWatchForKeyLocked(size_t handle);

  P_DISALLOW_COPY_AND_ASSIGN(FileSystemWatcherWin);
};

}  // namespace pixel
