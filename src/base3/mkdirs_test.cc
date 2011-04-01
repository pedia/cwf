#include <gtest/gtest.h>

#include "base3/mkdirs.h"

TEST(MkdirsTest, Noname) {
	using namespace base;
#if defined(OS_LINUX)
  int mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
  const char* arr[] = {
    "e/f/g/h",
    "./a/b",
    "./c/d/",
    "./d",
    0
  };
#else
  int mode = 0;
  const char* arr[] = {
    "e\\f\\g\\h",
    ".\\a\\b",
    ".\\c\\d\\",
    ".\\d",
    0
  };
#endif
  
  for(const char* p = arr[0]; *p; ++p)
    mkdirs(p, mode);
}
