#include "filesystem_watcher.h"

#include "platform.h"

#if P_OS_WIN
#include "filesystem_watcher_win.h"
#endif  // P_OS_WIN

namespace pixel {

FileSystemWatcher& FileSystemWatcher::ForProcess() {
#if P_OS_WIN
  static FileSystemWatcherWin watcher;
  return watcher;
#else
#error This platform has no FS watcher.
#endif
}

FileSystemWatcher::FileSystemWatcher() = default;

FileSystemWatcher::~FileSystemWatcher() = default;

bool FileSystemWatcher::StopWatchingForUpdates(std::optional<size_t> handle) {
  if (handle.has_value()) {
    return StopWatchingForUpdates(handle.value());
  }
  return false;
}

}  // namespace pixel
