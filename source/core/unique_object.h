#pragma once

#include <algorithm>
#include <utility>

namespace pixel {

template <class ObjectType, class ObjectTraits>
class UniqueObject {
 public:
  UniqueObject(ObjectType object) : object_(object) {}

  UniqueObject(UniqueObject&& object) { std::swap(object_, object.object_); }

  ~UniqueObject() {
    if (IsValid()) {
      ObjectTraits::Collect(object_);
    }
  }

  bool IsValid() const { return ObjectTraits::IsValid(object_); }

  const ObjectType& Get() const { return object_; }

 private:
  ObjectType object_ = ObjectTraits::DefaultValue();

  P_DISALLOW_COPY_AND_ASSIGN(UniqueObject);
};

}  // namespace pixel
