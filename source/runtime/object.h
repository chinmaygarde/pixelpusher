#pragma once

#include "macros.h"

namespace pixel {

// TODO: Remove this.
struct NullPeer {};

template <class _CType, class _PeerType = NullPeer>
class Object {
 public:
  using CType = _CType;
  using PeerType = _PeerType;

  static Object<CType, PeerType>* New(PeerType peer = {}) {
    return new Object<CType, PeerType>(std::move(peer));
  }

  CType* Get() { return &ffi_object_; }

  PeerType& GetPeer() { return peer_; }

  operator CType*() { return Get(); }

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
  CType ffi_object_ = {};
  const PeerType peer_;
  size_t ref_count_ = 1;

  Object(PeerType peer) : peer_(std::move(peer)) {
    ffi_object_.ffi_peer = this;
  }

  // Must use reference counting.
  ~Object() = default;

  P_DISALLOW_COPY_AND_ASSIGN(Object);
};

template <class CType>
Object<CType>* ToObject(CType* object) {
  return reinterpret_cast<Object<CType>*>(object->ffi_peer);
}

template <class _CType, class _PeerType = NullPeer>
class AutoObject {
 public:
  using CType = _CType;
  using PeerType = _PeerType;
  using ObjectType = Object<CType, PeerType>;

  static AutoObject Create(PeerType peer = {}) {
    AutoObject<CType, PeerType> object;
    object.Reset(ObjectType::New(std::move(peer)), true);
    return object;
  }

  AutoObject() = default;

  AutoObject(const AutoObject& other) { Reset(other.object_); }

  AutoObject(AutoObject&& other) { std::swap(other.object_, object_); }

  ~AutoObject() { Reset(); }

  AutoObject& operator=(const AutoObject& other) {
    Reset(other.object_);
    return *this;
  }

  AutoObject& operator=(AutoObject&& other) {
    std::swap(other.object_, object_);
    return *this;
  }

  void Reset(ObjectType* other = nullptr, bool adopt = false) {
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

  [[nodiscard]] ObjectType* TakeOwnership() {
    auto object = object_;
    object_ = nullptr;
    return object;
  }

  ObjectType* operator->() const { return object_; }

  ObjectType* Get() const { return object_; }

  bool IsValid() const { return object_ != nullptr; }

 private:
  ObjectType* object_ = nullptr;
};

}  // namespace pixel
