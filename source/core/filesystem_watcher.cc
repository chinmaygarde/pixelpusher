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

// FileSystemWatcher::FileSystemWatcher(std::unique_ptr<FileSystemWatcher>
// watcher)
//     : impl_(std::move(watcher)) {}

// FileSystemWatcher::~FileSystemWatcher() = default;

// std::optional<size_t> FileSystemWatcher::WatchPathForUpdates(
//     std::string path,
//     Closure change_callback) {
//   return impl_->WatchPathForUpdates(std::move(path),
//                                     std::move(change_callback));
// }

// bool FileSystemWatcher::StopWatchForUpdates(size_t handle) {
//   return impl_->StopWatchForUpdates(handle);
// }

}  // namespace pixel
