#ifndef PLATE_FSLOCK_H__
#define PLATE_FSLOCK_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef OS_LINUX
#include <unistd.h>
#elif defined(OS_WIN)
#include <io.h>
#endif

#include <string>
#include "base3/common.h"

namespace plate {

struct FileLock {
  FileLock(const char* name, bool lock = false) 
    : name_(name), locked_(false) {
      if (lock)
        TryLock();
  }
  ~FileLock() {
    if (locked_)
      Unlock();
  }
  bool Lock() {
    int delay = 5;
    do {
      locked_ = Lock(name_.c_str());
      if (!locked_) {
        base::Sleep(delay);
        delay += delay;
      }
    } while(!locked_);
    return locked_;
  }
  bool TryLock() {
    ASSERT(!locked_);
    locked_ = Lock(name_.c_str());
    return locked_;
  }
  void Unlock() {
    // ASSERT(locked_);
    if (locked_)
      Unlock(name_.c_str());
  }
  bool IsLocked() const {
    return locked_;
  }
#ifdef OS_LINUX
  static bool Lock(const char* name) {
    // open with O_EXCL
    int fd = open(name, O_RDWR | O_EXCL | O_CREAT
      , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (-1 == fd )
      return false;

    close(fd);
    return true;
  }
  static bool Unlock(const char* name) {
    return 0 == unlink(name);
  }
#elif defined(WIN32)
  static bool Lock(const char* name) {
    int fd = _open(name, _O_CREAT| _O_EXCL, 0);
    if (-1 == fd)
      return false;
    _close(fd);
    return true;
  }
  static bool Unlock(const char* name) {
    return 0 == _unlink(name);
  }
#endif
private:
  std::string name_;
  bool locked_;
};

}
#endif // PLATE_FSLOCK_H__
