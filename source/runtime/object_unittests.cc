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

}  // namespace testing
}  // namespace pixel
