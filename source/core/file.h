#pragma once

#include "logging.h"
#include "macros.h"
#include "platform.h"
#include "unique_object.h"

namespace pixel {

struct FDTraits {
  static bool IsValid(int fd);

  static int DefaultValue();

  static void Collect(int fd);
};

struct MappingType {
  const void* mapping = nullptr;
  size_t size = 0;
};

struct MappingTraits {
  static bool IsValid(const MappingType& mapping);

  static MappingType DefaultValue();

  static void Collect(const MappingType& mapping);
};

using UniqueFD = UniqueObject<int, FDTraits>;
using UniqueMapping = UniqueObject<MappingType, MappingTraits>;

class Mapping;
std::unique_ptr<Mapping> OpenFile(const char* file_name);

}  // namespace pixel
