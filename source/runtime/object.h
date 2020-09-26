#pragma once

#include <memory>

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

}  // namespace pixel
