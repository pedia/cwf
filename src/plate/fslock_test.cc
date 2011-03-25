#include "gtest/gtest.h"

#include <boost/thread.hpp>

#include "base3/common.h"
#include "plate/fslock.h"

const char* fn = "own";

#if defined(OS_LINUX)
TEST(FileLock, Linux) {
  int fd = creat(fn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  ASSERT_TRUE(-1 != fd);
  close(fd);

  struct stat st;
  ASSERT_EQ(0, stat(fn, &st));

  EXPECT_EQ(-1, open(fn, O_RDWR | O_EXCL | O_CREAT 
                    , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
  // TODO: test errno

  EXPECT_EQ(0, unlink(fn));
}
#endif

TEST(FileLock, Impl) {
  using namespace plate;
  {
    FileLock a(fn, true);

    struct stat st;
    ASSERT_EQ(0, stat(fn, &st));
  }
  struct stat st;
  ASSERT_EQ(-1, stat(fn, &st));
}

volatile bool magic_ = false;
bool quit_ = false;
volatile int count_ = 0, failed_count_ = 0;

void Proc() {
  using namespace plate;
  while(!quit_) {
    FileLock a(fn, true);
    if (a.IsLocked()) {
      EXPECT_EQ(0, magic_);
      magic_ = true;

      ++ count_;
      // xce::Sleep(1);

      magic_ = false;
    }
    else
      ++ failed_count_;
  }
}

#if defined(OS_LINUX)
TEST(FileLock, Threads) {
  boost::thread_group g;
  for (int i=0; i<10; ++i)
    g.create_thread(&Proc);

  base::Sleep(5000);
  quit_ = true;
  g.join_all();

  std::cout << "locked: " << count_ 
	    << " failed: " << failed_count_
	    << std::endl;
}
#endif

// ext3 
// locked: 91500 failed: 3147682
// wait lock
// locked: 270675 failed: 0

// moose
// try lock
// locked: 1889 failed: 27061

// wait lock
// locked: 6776 failed: 0
// delay=10ms
// locked: 6871 failed: 0
