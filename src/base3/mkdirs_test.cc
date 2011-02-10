#include <gtest/gtest.h>
#include "mkdirs.h"
TEST(MkdirsTest, Noname) {
#ifdef OS_LINUX
	using namespace base;
  mkdirs("./a/b", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  mkdirs("./c/d/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  mkdirs("./d", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}
