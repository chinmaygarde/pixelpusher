#pragma once

#include "macros.h"

#include <atomic>
#include <memory>
#include <thread>

namespace pixel {

template <class T>
class UnsharedWeakFactory;

template <class T>
class UnsharedControlBlock {
 public:
  UnsharedControlBlock() = default;

  ~UnsharedControlBlock() {
    // The factory must invalidate the control block before termination.
    P_ASSERT(!is_valid_);
  }

  bool IsValid() const {
    P_ASSERT(thread_id_ == std::this_thread::get_id());
    return is_valid_;
  }

 private:
  friend class UnsharedWeakFactory<T>;

  std::thread::id thread_id_ = std::this_thread::get_id();
  std::atomic_bool is_valid_ = true;

  void Invalidate() {
    P_ASSERT(thread_id_ == std::this_thread::get_id());
    is_valid_ = false;
  }

  P_DISALLOW_COPY_AND_ASSIGN(UnsharedControlBlock);
};

template <class T>
class UnsharedWeak {
 public:
  UnsharedWeak() = default;

  T* get() const { return IsValid() ? object_ : nullptr; }

  operator T*() const { return get(); }

  operator bool() const { return IsValid(); }

  bool IsValid() const { return control_block_ && control_block_->IsValid(); }

 private:
  friend class UnsharedWeakFactory<T>;

  T* object_ = nullptr;
  std::shared_ptr<UnsharedControlBlock<T>> control_block_;

  UnsharedWeak(T* object,
               std::shared_ptr<UnsharedControlBlock<T>> control_block)
      : object_(object), control_block_(std::move(control_block)) {}
};

template <class T>
class UnsharedWeakFactory {
 public:
  using WeakPtrType = UnsharedWeak<T>;

  UnsharedWeakFactory(T* object)
      : object_(object),
        control_block_(std::make_unique<UnsharedControlBlock<T>>()) {}

  ~UnsharedWeakFactory() { control_block_->Invalidate(); }

  auto CreateWeakPtr() { return UnsharedWeak<T>(object_, control_block_); }

 private:
  T* object_;
  std::shared_ptr<UnsharedControlBlock<T>> control_block_;

  P_DISALLOW_COPY_AND_ASSIGN(UnsharedWeakFactory);
};

}  // namespace pixel
