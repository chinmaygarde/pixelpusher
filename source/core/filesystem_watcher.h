#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "closure.h"
#include "macros.h"

namespace pixel {

class FileSystemWatcher {
 public:
  static FileSystemWatcher& ForProcess();

  FileSystemWatcher();

  virtual ~FileSystemWatcher();

  virtual std::optional<size_t> WatchPathForUpdates(
      std::filesystem::path path,
      Closure change_callback) = 0;

  bool StopWatchingForUpdates(std::optional<size_t> handle);

  virtual bool StopWatchingForUpdates(size_t handle) = 0;

  virtual void Terminate() = 0;

 private:
  P_DISALLOW_COPY_AND_ASSIGN(FileSystemWatcher);
};

}  // namespace pixel
