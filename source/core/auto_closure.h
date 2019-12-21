#pragma once

#include <functional>

#include "macros.h"

namespace pixel {

using Closure = std::function<void(void)>;

class AutoClosure {
 public:
  AutoClosure(Closure closure) : closure_(closure) {}

  ~AutoClosure() {
    if (closure_) {
      closure_();
    }
  }

 private:
  Closure closure_;

  P_DISALLOW_COPY_AND_ASSIGN(AutoClosure);
};

}  // namespace pixel
