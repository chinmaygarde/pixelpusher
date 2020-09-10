#include "file.h"

#if P_OS_WINDOWS

#include <Windows.h>

#include <codecvt>
#include <locale>
#include <sstream>
#include <string>

#include "win_utils.h"

#else  // P_OS_WINDOWS

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#endif  // // P_OS_WINDOWS

#include "logging.h"
#include "mapping.h"

namespace pixel {

class FileMapping : public Mapping {
 public:
  struct Data {
    const uint8_t* mapping = nullptr;
    size_t size = 0;
#if P_OS_WINDOWS
    UniqueFD fd;
    UniqueFD mapping_fd;
#endif  // P_OS_WINDOWS
  };

  FileMapping(Data data) : data_(std::move(data)) {}

  // |Mapping|
  ~FileMapping() override {
#if P_OS_WINDOWS
    if (!::UnmapViewOfFile(data_.mapping)) {
      P_ERROR << "Could not unmap view of file.";
    }
#else  // P_OS_WINDOWS
    if (::munmap(const_cast<uint8_t*>(data_.mapping), data_.size) != 0) {
      P_ERROR << "Could not unmap data.";
    }
#endif  // P_OS_WINDOWS
  }

  // |Mapping|
  const uint8_t* GetData() const override { return data_.mapping; }

  // |Mapping|
  size_t GetSize() const override { return data_.size; }

 private:
  Data data_;

  P_DISALLOW_COPY_AND_ASSIGN(FileMapping);
};

bool FDTraits::IsValid(Handle fd) {
#if P_OS_WINDOWS
  return fd != INVALID_HANDLE_VALUE;
#else  // P_OS_WINDOWS
  return fd >= 0;
#endif  // P_OS_WINDOWS
}

FDTraits::Handle FDTraits::DefaultValue() {
#if P_OS_WINDOWS
  return INVALID_HANDLE_VALUE;
#else  // P_OS_WINDOWS
  return -1;
#endif  // P_OS_WINDOWS
}

void FDTraits::Collect(Handle fd) {
#if P_OS_WINDOWS
  auto closed = ::CloseHandle(fd);
  if (!closed) {
    P_ERROR << "Could not close file handle.";
  }
#else  // P_OS_WINDOWS
  P_TEMP_FAILURE_RETRY(::close(fd));
#endif  // P_OS_WINDOWS
}

std::unique_ptr<Mapping> OpenFile(const std::filesystem::path& file_path) {
  if (file_path.empty()) {
    P_ERROR << "File path was empty.";
    return nullptr;
  }

#if P_OS_WINDOWS
  UniqueFD fd(::CreateFileW(file_path.c_str(),  // lpFileName
                            GENERIC_READ,       // dwDesiredAccess
                            FILE_SHARE_READ | FILE_SHARE_WRITE,  // dwShareMode
                            nullptr,                // lpSecurityAttributes
                            OPEN_EXISTING,          // dwCreationDisposition
                            FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAttributes
                            nullptr                 // hTemplateFile
                            ));
  if (!fd.IsValid()) {
    P_ERROR << "Could not open FD for file: " << file_path
            << " error: " << GetLastErrorMessage();
    return nullptr;
  }

  const auto mapping_size = ::GetFileSize(fd.Get(), nullptr);
  if (mapping_size == INVALID_FILE_SIZE) {
    P_ERROR << "Could not find file size.";
    return nullptr;
  }

  UniqueFD mapping_fd(::CreateFileMapping(fd.Get(),       // hFile
                                          nullptr,        // lpAttributes
                                          PAGE_READONLY,  // flProtect
                                          0,              // dwMaximumSizeHigh
                                          0,              // dwMaximumSizeLow
                                          nullptr         // lpName
                                          ));

  if (!mapping_fd.IsValid()) {
    P_ERROR << "Could not setup file mapping: " << GetLastErrorMessage();
    return nullptr;
  }

  auto mapping = reinterpret_cast<uint8_t*>(MapViewOfFile(
      mapping_fd.Get(),  // hFileMappingObject
      FILE_MAP_READ,     // hFileMappingObject
      0,                 // hFileMappingObject
      0,                 // hFileMappingObject
      mapping_size       // hFileMappingObject
      ));

  if (mapping == nullptr) {
    P_ERROR << "Could not map file: " << GetLastErrorMessage();
    return nullptr;
  }

  FileMapping::Data mapping_data;
  mapping_data.mapping = mapping;
  mapping_data.size = mapping_size;
  mapping_data.fd = std::move(fd);
  mapping_data.mapping_fd = std::move(mapping_fd);
  return std::make_unique<FileMapping>(std::move(mapping_data));

#else  // P_OS_WINDOWS
  UniqueFD fd(
      P_TEMP_FAILURE_RETRY(::open(file_name.c_str(), O_RDONLY | O_CLOEXEC)));

  if (!fd.IsValid()) {
    P_ERROR << "Could not open FD.";
    return nullptr;
  }

  struct stat stat_buf = {};

  if (::fstat(fd.Get(), &stat_buf) != 0) {
    P_ERROR << "Could not stat file.";
    return nullptr;
  }

  auto mapping_ptr = ::mmap(nullptr, stat_buf.st_size, PROT_READ,
                            MAP_FILE | MAP_PRIVATE, fd.Get(), 0);
  if (mapping_ptr == MAP_FAILED) {
    P_ERROR << "Could not mmap file descriptor.";
    return nullptr;
  }

  FileMapping::Data mapping_data;
  mapping_data.mapping = reinterpret_cast<uint8_t*>(mapping_ptr);
  mapping_data.size = stat_buf.st_size;
  return std::make_unique<FileMapping>(std::move(mapping_data));
#endif  // P_OS_WINDOWS
}

std::unique_ptr<Mapping> OpenFile(const char* file_name) {
  return OpenFile(std::filesystem::path{file_name});
}

std::filesystem::path GetCurrentExecutablePath() {
#if P_OS_WINDOWS
  auto module_handle = ::GetModuleHandle(nullptr);
  P_ASSERT(module_handle);
  char path[MAX_PATH];
  auto read_size = ::GetModuleFileNameA(module_handle, path, MAX_PATH);
  P_ASSERT(read_size > 0);
  return {std::string{path, read_size}};
#else
#error Not currently supported on this platform.
  return {};
#endif
}

}  // namespace pixel
