#include <cstdlib>

#include <gtest/gtest.h>

TEST(GoogleTest, Add)
{
  EXPECT_STREQ(std::getenv("VALX"), "1");
  EXPECT_STREQ(std::getenv("VALY"), "2");
}
