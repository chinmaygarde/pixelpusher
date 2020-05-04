
#include <gtest/gtest.h>

#include "file.h"
#include "fixture.h"

namespace pixel {
namespace testing {

TEST(File, CanMemoryMapFile) {
  auto hello = OpenFile(P_FIXTURES_LOCATION "hello.txt");
  ASSERT_TRUE(hello);

  auto does_not_exist = OpenFile(P_FIXTURES_LOCATION "does_not_exist.txt");
  ASSERT_FALSE(does_not_exist);
}

}  // namespace testing
}  // namespace pixel
