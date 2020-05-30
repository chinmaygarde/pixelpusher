#include "closure.h"

#include <atomic>

namespace pixel {

size_t IdentifiableCallback::GetNextCallbackID() {
  static std::atomic sLastHandle = 0;
  return ++sLastHandle;
}

}  // namespace pixel
