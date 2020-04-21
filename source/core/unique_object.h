#pragma once

#include <algorithm>
#include <utility>

namespace pixel {

template <class ObjectType, class ObjectTraits>
class UniqueObject {
 public:
  UniqueObject(ObjectType object = ObjectTraits::DefaultValue())
      : object_(object) {}

  UniqueObject(UniqueObject&& object) { std::swap(object_, object.object_); }

  ~UniqueObject() { Reset(); }

  UniqueObject& operator=(UniqueObject&& other) {
    Reset(other.Release());
    return *this;
  }

  bool IsValid() const { return ObjectTraits::IsValid(object_); }

  const ObjectType& Get() const { return object_; }

  void Reset(ObjectType value = ObjectTraits::DefaultValue()) {
    if (object_ == value) {
      return;
    }
    if (IsValid()) {
      ObjectTraits::Collect(object_);
    }
    object_ = value;
  }

  [[nodiscard]] ObjectType Release() {
    auto old = object_;
    object_ = ObjectTraits::DefaultValue();
    return old;
  }

 private:
  ObjectType object_ = ObjectTraits::DefaultValue();

  P_DISALLOW_COPY_AND_ASSIGN(UniqueObject);
};

}  // namespace pixel
