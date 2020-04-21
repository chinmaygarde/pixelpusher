#pragma once

#include "logging.h"
#include "macros.h"
#include "platform.h"
#include "unique_object.h"

namespace pixel {

struct FDTraits {
#if P_OS_WINDOWS
  using Handle = void*;
#else   // P_OS_WINDOWS
  using Handle = int;
#endif  // P_OS_WINDOWS

  static bool IsValid(Handle fd);

  static Handle DefaultValue();

  static void Collect(Handle fd);
};

using UniqueFD = UniqueObject<FDTraits::Handle, FDTraits>;

class Mapping;
std::unique_ptr<Mapping> OpenFile(const char* file_name);

}  // namespace pixel
