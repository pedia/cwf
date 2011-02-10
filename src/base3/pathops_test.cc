#include <gtest/gtest.h>
#include "base3/pathops.h"

using namespace base;

TEST(Pathops, Dirname) {
  EXPECT_EQ("/a", Dirname("/a/b"));
  EXPECT_EQ("/a/b", Dirname("/a/b/"));
  EXPECT_EQ("a", Dirname("a/b"));
  EXPECT_EQ("", Dirname("a"));
}
