#include <gtest/gtest.h>
#include "object.h"

namespace pixel {
namespace testing {

TEST(ObjectTest, CanCreateObjects) {
  static bool created = false;
  static bool deleted = false;
  created = false;
  deleted = false;
  struct MyStruct {
    void* ffi_peer;

    MyStruct() { created = true; }
    ~MyStruct() { deleted = true; }
  };

  ASSERT_FALSE(created);
  ASSERT_FALSE(deleted);
  auto object = Object<MyStruct>::New();
  ASSERT_NE(object->GetFFIObject(), nullptr);
  ASSERT_NE(static_cast<MyStruct*>(*object), nullptr);
  ASSERT_TRUE(created);
  ASSERT_FALSE(deleted);
  ASSERT_EQ(object->GetRefCount(), 1u);
  object->Retain();
  ASSERT_TRUE(created);
  ASSERT_FALSE(deleted);
  ASSERT_EQ(object->GetRefCount(), 2u);
  object->Release();
  ASSERT_TRUE(created);
  ASSERT_FALSE(deleted);
  ASSERT_EQ(object->GetRefCount(), 1u);
  object->Release();
  ASSERT_TRUE(created);
  ASSERT_TRUE(deleted);
}

TEST(ObjectTest, AutoObjectsWork) {
  static bool created = false;
  static bool deleted = false;
  created = false;
  deleted = false;
  struct MyStruct {
    void* ffi_peer;

    MyStruct() { created = true; }
    ~MyStruct() { deleted = true; }
  };

  {
    ASSERT_FALSE(created);
    ASSERT_FALSE(deleted);
    auto object = AutoObject<MyStruct>::Create();
    ASSERT_TRUE(created);
    ASSERT_FALSE(deleted);
    ASSERT_EQ(object->GetRefCount(), 1u);
    {
      auto object_copy = object;
      ASSERT_TRUE(created);
      ASSERT_FALSE(deleted);
      ASSERT_EQ(object->GetRefCount(), 2u);
      ASSERT_EQ(object_copy->GetRefCount(), 2u);
      ASSERT_EQ(object.Get(), object_copy.Get());
    }
    ASSERT_EQ(object->GetRefCount(), 1u);
  }

  ASSERT_TRUE(created);
  ASSERT_TRUE(deleted);

  created = false;
  deleted = false;

  {
    auto object = AutoObject<MyStruct>::Create();
    ASSERT_TRUE(created);
    ASSERT_FALSE(deleted);
    ASSERT_EQ(object->GetRefCount(), 1u);
    auto copy_assigned_object = object;
    ASSERT_EQ(object->GetRefCount(), 2u);
    ASSERT_EQ(copy_assigned_object->GetRefCount(), 2u);
    ASSERT_EQ(object.Get(), copy_assigned_object.Get());
  }

  ASSERT_TRUE(created);
  ASSERT_TRUE(deleted);

  created = false;
  deleted = false;

  {
    auto object = AutoObject<MyStruct>::Create();
    ASSERT_TRUE(created);
    ASSERT_FALSE(deleted);
    ASSERT_EQ(object->GetRefCount(), 1u);
    auto move_assigned_object = std::move(object);
    ASSERT_EQ(object.Get(), nullptr);
    ASSERT_EQ(move_assigned_object->GetRefCount(), 1u);
  }

  ASSERT_TRUE(created);
  ASSERT_TRUE(deleted);

  created = false;
  deleted = false;
  {
    auto object = AutoObject<MyStruct>::Create();
    ASSERT_TRUE(created);
    ASSERT_FALSE(deleted);
    ASSERT_EQ(object->GetRefCount(), 1u);
    auto released_ptr = object.Release();
    object.Reset();
    ASSERT_TRUE(created);
    ASSERT_FALSE(deleted);
    AutoObject<MyStruct> object_via_adopt;
    object_via_adopt.Reset(released_ptr, true);
    ASSERT_EQ(object_via_adopt->GetRefCount(), 1u);
  }
  ASSERT_TRUE(created);
  ASSERT_TRUE(deleted);
}

}  // namespace testing
}  // namespace pixel
