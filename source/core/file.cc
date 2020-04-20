#include "file.h"

#if P_OS_WINDOWS
#else  // P_OS_WINDOWS
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif  // // P_OS_WINDOWS

#include "logging.h"
#include "mapping.h"

namespace pixel {

bool FDTraits::IsValid(int fd) {
  return fd >= 0;
}

int FDTraits::DefaultValue() {
  return -1;
}

void FDTraits::Collect(int fd) {
  P_TEMP_FAILURE_RETRY(::close(fd));
}

bool MappingTraits::IsValid(const MappingType& mapping) {
  return mapping.mapping != nullptr;
}

MappingType MappingTraits::DefaultValue() {
  return {};
}

void MappingTraits::Collect(const MappingType& mapping) {
  auto result = ::munmap(const_cast<void*>(mapping.mapping), mapping.size);
  if (result != 0) {
    P_ERROR << "Could not release memory mapping.";
  }
}

std::unique_ptr<Mapping> OpenFile(const char* file_name) {
  UniqueFD fd(P_TEMP_FAILURE_RETRY(::open(file_name, O_RDONLY | O_CLOEXEC)));
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

  MappingType mapping_type;
  mapping_type.mapping = mapping_ptr;
  mapping_type.size = stat_buf.st_size;

  return std::make_unique<MemoryMapping>(mapping_type);
}

}  // namespace pixel
