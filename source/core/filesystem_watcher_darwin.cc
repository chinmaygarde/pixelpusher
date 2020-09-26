#include "filesystem_watcher_darwin.h"

namespace pixel {

FileSystemWatcherDarwin::FileSystemWatcherDarwin() = default;

// |FileSystemWatcher|
FileSystemWatcherDarwin::~FileSystemWatcherDarwin() = default;

// |FileSystemWatcher|
std::optional<size_t> FileSystemWatcherDarwin::WatchPathForUpdates(
    std::filesystem::path path,
    Closure change_callback) {
  return std::nullopt;
}

// |FileSystemWatcher|
bool FileSystemWatcherDarwin ::StopWatchingForUpdates(size_t handle) {
  return false;
}

// |FileSystemWatcher|
void FileSystemWatcherDarwin::Terminate() {
  //
}

}  // namespace pixel
