#include <gtest/gtest.h>

#include "unshared_weak.h"

namespace pixel {

TEST(UnsharedWeakTest, CanCreateFactory) {
  int a = 0;
  UnsharedWeakFactory<int> aWeakFactory(&a);
  auto weak = aWeakFactory.CreateWeakPtr();
  ASSERT_TRUE(weak);
}

TEST(UnsharedWeakTest, FactoryCollectionInvalidatesControlBlock) {
  int a = 122;

  UnsharedWeak<int> weak;
  ASSERT_FALSE(weak);
  {
    UnsharedWeakFactory<int> aWeakFactory(&a);
    weak = aWeakFactory.CreateWeakPtr();
    ASSERT_TRUE(weak);
    ASSERT_EQ(weak.get(), &a);
  }
  ASSERT_FALSE(weak);
  ASSERT_EQ(weak.get(), nullptr);
}

}  // namespace pixel
