#include "filesystem_watcher.h"

#include "platform.h"

#if P_OS_WIN
#include "filesystem_watcher_win.h"
#elif P_OS_MAC
#include "filesystem_watcher_darwin.h"
#endif

namespace pixel {

FileSystemWatcher& FileSystemWatcher::ForProcess() {
#if P_OS_WIN
  static FileSystemWatcherWin watcher;
  return watcher;
#elif P_OS_MAC
  static FileSystemWatcherDarwin watcher;
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
