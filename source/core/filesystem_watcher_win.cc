#include "filesystem_watcher_win.h"

#include <vector>

#include "closure.h"
#include "logging.h"
#include "win_utils.h"

namespace pixel {

class PendingFileSystemWatch {
 public:
  PendingFileSystemWatch(const std::filesystem::path& path,
                         std::filesystem::path file_name,
                         const UniqueFD& completion_port,
                         size_t iocp_completion_key,
                         Closure callback)
      : file_name_(std::move(file_name)), callback_(callback) {
    notify_info_buffer_.resize(5 * sizeof(FILE_NOTIFY_INFORMATION));

    dir_fd_.Reset(::CreateFileW(
        path.c_str(),         // lpFileName
        FILE_LIST_DIRECTORY,  // dwDesiredAccess
        FILE_SHARE_READ,      // dwShareMode
        nullptr,              // lpSecurityAttributes
        OPEN_EXISTING,        // dwCreationDisposition
        FILE_FLAG_BACKUP_SEMANTICS |
            FILE_FLAG_OVERLAPPED,  // dwFlagsAndAttributes
        nullptr                    // hTemplateFile
        ));

    if (!dir_fd_.IsValid()) {
      P_ERROR << "Could not open directory " << path << " "
              << GetLastErrorMessage();
      return;
    }

    auto result = ::CreateIoCompletionPort(
        dir_fd_.Get(),          // handle
        completion_port.Get(),  // existing completion port
        iocp_completion_key,    // completion key
        1u                      // concurrent threads
    );

    if (result != completion_port.Get()) {
      P_ERROR << "Could not setup a completion port for " << path << " "
              << GetLastErrorMessage();
      return;
    }

    event_.Reset(::CreateEventW(nullptr,  // lpEventAttributes
                                true,     // bManualReset
                                false,    // bInitialState
                                nullptr   // lpName
                                ));
    if (!event_.IsValid()) {
      P_ERROR << "Could not create event for " << path << " "
              << GetLastErrorMessage();
      return;
    }

    overlapped_.hEvent = event_.Get();

    if (!StartNewWatch()) {
      P_ERROR << "Could not start new watch.";
    }

    is_valid_ = true;
  }

  bool OnWatchDidFinish(size_t bytes_transferred) {
    std::vector<uint8_t> updated_notifications(
        notify_info_buffer_.begin(),
        notify_info_buffer_.begin() + bytes_transferred);
    return StartNewWatch() &&
           DispatchNotifications(std::move(updated_notifications));
  }

  bool DispatchNotifications(std::vector<uint8_t> updates) const {
    for (size_t current_offset = 0; current_offset < updates.size();) {
      auto notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(updates.data() +
                                                               current_offset);
      current_offset += notify->NextEntryOffset;

      if (notify->Action == FILE_ACTION_MODIFIED) {
        std::wstring file_name(notify->FileName,
                               notify->FileNameLength / sizeof(wchar_t));
        std::filesystem::path file_path(file_name);
        if (file_path == file_name_) {
          callback_();
        }
      }

      if (notify->NextEntryOffset == 0) {
        break;
      }
    }
    return true;
  }

  bool StartNewWatch() {
    // Unused because we are using asynchronous notifications.
    DWORD unused_bytes_returned = 0;

    if (!::ResetEvent(event_.Get())) {
      P_ERROR << "Could not reset event: " << GetLastErrorMessage();
      return false;
    }

    if (!::ReadDirectoryChangesW(
            dir_fd_.Get(),                  // hDirectory
            notify_info_buffer_.data(),     // lpBuffer
            notify_info_buffer_.size(),     // nBufferLength
            false,                          // bWatchSubtree
            FILE_NOTIFY_CHANGE_LAST_WRITE,  // dwNotifyFilter
            &unused_bytes_returned,         // lpBytesReturned
            &overlapped_,                   // lpOverlapped
            nullptr                         // lpCompletionRoutine
            )) {
      P_ERROR << "Could not watch for directory changes: "
              << GetLastErrorMessage();
      return false;
    }

    return true;
  }

  ~PendingFileSystemWatch() = default;

  bool IsValid() const { return is_valid_; }

 private:
  UniqueFD dir_fd_;
  std::vector<uint8_t> notify_info_buffer_;
  UniqueFD event_;
  OVERLAPPED overlapped_ = {};
  std::filesystem::path file_name_;
  Closure callback_;
  bool is_valid_ = false;

  P_DISALLOW_COPY_AND_ASSIGN(PendingFileSystemWatch);
};

FileSystemWatcherWin::FileSystemWatcherWin()
    : completion_port_(::CreateIoCompletionPort(
          INVALID_HANDLE_VALUE,                  // handle
          NULL,                                  // existing completion port
          reinterpret_cast<ULONG_PTR>(nullptr),  // completion key
          1u                                     // concurrent threads
          )),
      thread_([&]() { WatcherMain(); }) {}

FileSystemWatcherWin::~FileSystemWatcherWin() {
  Terminate();
}

std::optional<size_t> FileSystemWatcherWin::WatchPathForUpdates(
    std::filesystem::path path,
    Closure change_callback) {
  if (path.empty() || !path.has_parent_path() || change_callback == nullptr) {
    return std::nullopt;
  }

  path.make_preferred();

  auto directory = path.parent_path();
  auto file_name = path.filename();

  auto new_handle = ++last_handle_;
  auto watch = std::make_unique<PendingFileSystemWatch>(directory,         //
                                                        file_name,         //
                                                        completion_port_,  //
                                                        new_handle,        //
                                                        change_callback    //
  );

  if (!watch->IsValid()) {
    P_ERROR << "Could not setup watcher for " << path;
    return std::nullopt;
  }

  std::scoped_lock lock(mutex_);
  watches_[new_handle] = std::move(watch);
  return new_handle;
}

bool FileSystemWatcherWin::StopWatchingForUpdates(size_t handle) {
  std::scoped_lock lock(mutex_);
  auto found = watches_.find(handle);
  if (found == watches_.end()) {
    return false;
  }
  // Closing the watch will close the handle which will also abandon the wait.
  watches_.erase(found);
  return true;
}

void FileSystemWatcherWin::WatcherMain() {
  if (!completion_port_.IsValid()) {
    P_ERROR << "Completion port for the filesystem watcher was invalid. Expect "
               "no more filesystem updates.";
    return;
  }

  while (!terminated_) {
    DWORD bytes_transferred = 0;
    ULONG_PTR completion_key = 0;
    LPOVERLAPPED overlapped_ptr = nullptr;
    const auto result = ::GetQueuedCompletionStatus(
        completion_port_.Get(),  // CompletionPort
        &bytes_transferred,      // lpNumberOfBytesTransferred
        &completion_key,         // lpCompletionKey
        &overlapped_ptr,         // lpOverlapped
        INFINITE                 // dwMilliseconds
    );
    if (!result) {
      // A port may be closed.
      continue;
    }
    if (completion_key != 0) {
      std::scoped_lock lock(mutex_);
      auto watch = GetWatchForKeyLocked(completion_key);
      // The watch may have ended if the watcher was removed on another thread.
      if (watch) {
        if (!watch->OnWatchDidFinish(bytes_transferred)) {
          P_ERROR
              << "Could not finish watch on filesystem handle. Stopping all "
                 "watches. Expect no more filesystem updates.";
          Terminate();
          return;
        }
      }
    }
  }
}

// |FileSystemWatcher|
void FileSystemWatcherWin::Terminate() {
  std::scoped_lock lock(mutex_);
  terminated_ = true;
  // This will close handles which will also abandon the wait.
  watches_.clear();
}

PendingFileSystemWatch* FileSystemWatcherWin::GetWatchForKeyLocked(
    size_t handle) {
  auto found = watches_.find(handle);
  if (found == watches_.end()) {
    return nullptr;
  }
  return found->second.get();
}

}  // namespace pixel
