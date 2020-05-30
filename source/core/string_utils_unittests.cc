#include <gtest/gtest.h>

#include "string_utils.h"

namespace pixel {
namespace testing {

TEST(StringTest, TestTrim) {
  {
    auto string = "   Hello  World   ";
    auto stripped = TrimString(string);
    ASSERT_EQ(stripped, "Hello  World");
  }

  {
    auto string = "";
    auto stripped = TrimString(string);
    ASSERT_EQ(stripped, "");
  }

  {
    auto string = " ";
    auto stripped = TrimString(string);
    ASSERT_EQ(stripped, "");
  }

  {
    auto string = "AsIs";
    auto stripped = TrimString(string);
    ASSERT_EQ(stripped, "AsIs");
  }

  {
    auto string = "   \n   ";
    auto stripped = TrimString(string);
    ASSERT_EQ(stripped, "");
  }

  {
    auto string = R"~(

      Hello

    )~";
    auto stripped = TrimString(string);
    ASSERT_EQ(stripped, "Hello");
  }

  {
    auto string = R"~(

      Hello

      World

    )~";
    auto stripped = TrimString(string);
    ASSERT_EQ(stripped, R"~(Hello

      World)~");
  }
}

}  // namespace testing
}  // namespace pixel
