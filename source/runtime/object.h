#pragma once

#include "macros.h"

namespace pixel {

template <class T>
class Object {
 public:
  using FFIType = T;

  static Object<FFIType>* New() { return new Object<FFIType>(); }

  FFIType* GetFFIObject() { return &ffi_object_; }

  operator FFIType*() { return GetFFIObject(); }

  void Retain() { ref_count_++; }

  void Release() {
    P_ASSERT(ref_count_ > 0u);
    ref_count_--;
    if (ref_count_ == 0) {
      delete this;
    }
  }

  size_t GetRefCount() const { return ref_count_; }

 private:
  FFIType ffi_object_ = {};
  size_t ref_count_ = 1;

  Object() { ffi_object_.ffi_peer = this; }

  // Must use reference counting.
  ~Object() = default;

  P_DISALLOW_COPY_AND_ASSIGN(Object);
};

template <class FFIType>
Object<FFIType>* ToObject(FFIType* object) {
  return reinterpret_cast<Object<FFIType>*>(object->ffi_peer);
}

template <class T>
class AutoObject {
 public:
  static AutoObject Create() {
    AutoObject<T> object;
    object.Reset(Object<T>::New(), true);
    return object;
  }

  AutoObject() = default;

  AutoObject(const AutoObject& other) { Reset(other.object_); }

  AutoObject(AutoObject&& other) { std::swap(other.object_, object_); }

  ~AutoObject() { Reset(); }

  AutoObject& operator=(const AutoObject& other) { Reset(other.object_); }

  AutoObject& operator=(AutoObject&& other) {
    std::swap(other.object_, object_);
  }

  void Reset(Object<T>* other = nullptr, bool adopt = false) {
    if (object_) {
      object_->Release();
      object_ = nullptr;
    }

    if (other) {
      if (!adopt) {
        other->Retain();
      }
      object_ = other;
    }
  }

  [[nodiscard]] Object<T>* Release() {
    auto object = object_;
    object_ = nullptr;
    return object;
  }

  Object<T>* operator->() const { return object_; }

  Object<T>* Get() const { return object_; }

 private:
  Object<T>* object_ = nullptr;
};

}  // namespace pixel
