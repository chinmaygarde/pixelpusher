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

struct IdentifiableCallback {
 public:
  static size_t GetNextCallbackID();

  IdentifiableCallback() = default;

  IdentifiableCallback(Closure closure)
      : identifier_(GetNextCallbackID()), closure_(closure) {}

  ~IdentifiableCallback() = default;

  constexpr size_t GetIdentifier() const { return identifier_; }

  template <class... ArgTypes>
  auto operator()(ArgTypes&&... args) const {
    return closure_(std::forward<ArgTypes>(args)...);
  };

 private:
  size_t identifier_ = 0;
  Closure closure_;
};

}  // namespace pixel
