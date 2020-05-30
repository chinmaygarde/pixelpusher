#pragma once

#include <functional>
#include <map>
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

size_t GetNextCallbackID();

template <class T>
struct IdentifiableCallback {
 public:
  IdentifiableCallback() = default;

  IdentifiableCallback(std::function<T> closure)
      : identifier_(GetNextCallbackID()), closure_(closure) {}

  ~IdentifiableCallback() = default;

  constexpr size_t GetIdentifier() const { return identifier_; }

  template <class... ArgTypes>
  auto operator()(ArgTypes&&... args) const {
    return closure_(std::forward<ArgTypes>(args)...);
  };

 private:
  size_t identifier_ = 0;
  std::function<T> closure_;
};

using IdentifiableClosure = IdentifiableCallback<void(void)>;

template <class T>
auto MakeIdentifiable(std::function<T> function) {
  return IdentifiableCallback<T>(function);
}

template <class T>
using IdentifiableCallbacks = std::map<size_t, std::function<T>>;

using IdentifiableClosures = IdentifiableCallbacks<void(void)>;

template <class T>
size_t IdentifiableCallbacksAdd(IdentifiableCallbacks<T>& callbacks,
                                std::function<T> callback) {
  auto id_callback = MakeIdentifiable(callback);
  const auto handle = id_callback.GetIdentifier();
  callbacks[handle] = std::move(id_callback);
  return handle;
}

template <class T>
bool IdentifiableCallbacksRemove(IdentifiableCallbacks<T>& callbacks,
                                 size_t handle) {
  return callbacks.erase(handle) == 1u;
}

template <class T, class... ArgTypes>
void IdentifiableCallbacksInvoke(const IdentifiableCallbacks<T>& callbacks,
                                 ArgTypes&&... args) {
  for (const auto& callback : callbacks) {
    callback.second(args...);
  }
}

}  // namespace pixel
