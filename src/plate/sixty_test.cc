#include <gtest/gtest.h>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include "base3/common.h"
#include "plate/tenandsixty.h"

using namespace plate;

TEST(Sixty, All) {
  unsigned int arr[] = {
    0, 1, 10, 11, 30, 88, 99, 100, 101, 200, 400, 9999, 1000, 10001
    , 312312, 901238, 238432974, 783467234, 3948475566
    , -2, -1
  };
  for (int i=0; i<ARRAY_SIZE(arr); ++i) {
    std::string a = ToSixty(arr[i]);
    std::cout << std::setw(10) << arr[i] 
      << std::setw(8) << a << std::endl;

    EXPECT_LE(a.size(), boost::lexical_cast<std::string>(arr[i]).size());
    EXPECT_EQ(arr[i], FromSixty(a));
  }

  EXPECT_EQ(0, FromSixty(""));
  EXPECT_EQ("A", ToSixty(0));

  // 错误情况判断
  // 超大值
  EXPECT_EQ(-1, FromSixty("2394u234nkasdfasdfas234n2k34n23k4b"));
  // 比最大值大一QQOkhg5VQfH+1 为 QQOkhg5VQfJ
  EXPECT_EQ(-1, FromSixty("QQOkhg5VQfJ"));
  // 等于最大值
  EXPECT_EQ(9223372036854775807LL, FromSixty("QQOkhg5VQfH"));
  // 比最大值小1
  EXPECT_EQ(9223372036854775806LL, FromSixty("QQOkhg5VQfG"));
  // 输入整数为负数
  EXPECT_STREQ("-1", ToSixty(-293409234).c_str());
}

TEST(Sixty, Profiler) {
  const int test_count = 100 * 1024;
  {
    for (int i=0; i<test_count; ++i)
      ToSixty(i);
  }

  std::vector<std::string> vs;
  vs.reserve(test_count);
  for (int i=0; i<test_count; ++i) {
    vs.push_back(ToSixty(i));
  }

  {
    for (std::vector<std::string>::const_iterator i=vs.begin();
      i != vs.end(); ++i)
      FromSixty(*i);
  }
}
