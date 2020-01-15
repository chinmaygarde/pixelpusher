#pragma once

#include <sys/mman.h>
#include <unistd.h>

#include "logging.h"
#include "macros.h"
#include "unique_object.h"

namespace pixel {

struct FDTraits {
  static bool IsValid(int fd) { return fd >= 0; }

  static int DefaultValue() { return -1; }

  static void Collect(int fd) { P_TEMP_FAILURE_RETRY(::close(fd)); }
};

struct MappingType {
  const void* mapping = nullptr;
  size_t size = 0;
};

struct MappingTraits {
  static bool IsValid(const MappingType& mapping) {
    return mapping.mapping != nullptr;
  }

  static MappingType DefaultValue() { return {}; }

  static void Collect(const MappingType& mapping) {
    auto result = ::munmap(const_cast<void*>(mapping.mapping), mapping.size);
    if (result != 0) {
      P_ERROR << "Could not release memory mapping.";
    }
  }
};

using UniqueFD = UniqueObject<int, FDTraits>;
using UniqueMapping = UniqueObject<MappingType, MappingTraits>;

class Mapping;
std::unique_ptr<Mapping> OpenFile(const char* file_name);

}  // namespace pixel
