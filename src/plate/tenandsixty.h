#ifndef CELL_MFSCLIENT_TENANDSIXTY_H__
#define CELL_MFSCLIENT_TENANDSIXTY_H__

#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "base3/basictypes.h"

namespace plate {

inline std::string ToSixty(int64 number) {
  if (number < 0)
    return "-1";
  std::string result_str;
  // 这行代码能提升性能 1 倍多
  // ToSixy 100k clock: 36.545 ms
  // ToSixy 100k clock: 17.664 ms
  result_str.reserve(10);
  const int base = 60;

  const char SixtyChar[] = "ABCDEFGHJKLMNOPQRSTUVWXYZabcdefghjklmnopqrstuvwxyz0123456789";

  do {
    int m = number % base;
    result_str.push_back(SixtyChar[m]);
    number /= base;
  } while (number);

  std::reverse(result_str.begin(), result_str.end());

  return result_str;
}

// 支持 64 位整数
// int64最大正整数为9223372036854775807
// 60进制形式为QQOkhg5VQfH
// 如果错误，返回 -1
inline int64 FromSixty(const std::string& num_sixty) {
  static int CharTable[] = {
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2
    , 3, 4, 5, 6, 7, -1, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
    , 22, 23, 24, -1, -1, -1, -1, -1, -1, 25, 26, 27, 28, 29, 30, 31, 32, -1
    , 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49
  };
  static int64 PowTable[] = {
    1, 60, 3600, 216000, 12960000,
    777600000, 46656000000LL, 2799360000000LL,
    167961600000000LL, 10077696000000000LL, 604661760000000000LL
  };

  int size = num_sixty.size();
  // 如果超过了60进制最大值的位数11
  if (size > 11)
    return -1;

  int64 ten = 0;

  for (int j=0; j<size; j++) {
    char c = num_sixty[j];
    // 判断输入的六十进制是否合法
    if (c < '0' || c > 'z')
      return -1;

    // ten += index * pow(60, position)
    //     A => 0
    //     B => 1
    //     a => 26
    //     b => 27

    int index = CharTable[c - '0']; 
    ten += index * PowTable[size - j - 1];
    // 在此处做超过最大值判断，如果变成负数，就返回-1
    if (ten < 0)
      return -1;
  }

  return ten;
}

}
#endif // CELL_MFSCLIENT_TENANDSIXTY_H__
