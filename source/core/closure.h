#pragma once

#include <functional>
#include <memory>
#include <utility>

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

template <class T>
struct CopyableClosure {
 public:
  explicit CopyableClosure(T closure)
      : closure_(std::make_shared<T>(std::move(closure))) {}

  template <class... ArgTypes>
  auto operator()(ArgTypes&&... args) const {
    return (*closure_)(std::forward<ArgTypes>(args)...);
  };

 private:
  std::shared_ptr<T> closure_;
};

template <class T>
auto MakeCopyable(T closure) {
  return CopyableClosure<T>(std::move(closure));
}

}  // namespace pixel
