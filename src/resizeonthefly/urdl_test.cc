#include <gtest/gtest.h>

#include <string.h>

#include <fstream>
#include <vector>

#include "resizeonthefly/httpdownload.h"

using namespace rof;

TEST(Urdl, Download) {
  std::vector<char> buf;
  bool f = HttpDownload("http://www.google.com/", &buf);
  EXPECT_TRUE(f);
  EXPECT_GT(buf.size(), 100);

  buf.clear();
  f = HttpDownload("http://www.renren.com/", &buf);
  EXPECT_TRUE(f);
  EXPECT_GT(buf.size(), 1000);
}
