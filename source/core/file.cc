#include "file.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "logging.h"
#include "mapping.h"

namespace pixel {

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
