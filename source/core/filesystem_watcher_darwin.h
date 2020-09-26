#pragma once

#include "filesystem_watcher.h"
#include "macros.h"

namespace pixel {

class FileSystemWatcherDarwin final : public FileSystemWatcher {
 public:
  FileSystemWatcherDarwin();

  // |FileSystemWatcher|
  ~FileSystemWatcherDarwin() override;

  // |FileSystemWatcher|
  std::optional<size_t> WatchPathForUpdates(std::filesystem::path path,
                                            Closure change_callback) override;

  // |FileSystemWatcher|
  bool StopWatchingForUpdates(size_t handle) override;

  // |FileSystemWatcher|
  void Terminate() override;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(FileSystemWatcherDarwin);
};

}  // namespace pixel
