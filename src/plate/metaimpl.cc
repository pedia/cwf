#include "plate/metaimpl.h"

#include <errno.h>

#include "base3/pathops.h"
#include "base3/mkdirs.h"
#include "base3/logging.h"

#include "plate/fslock.h"

namespace plate {

// TODO: use kRoot
static const char* kLockNameFormat = "/mnt/mfs/.lock/%08x";

static volatile int last_id_ = 0;
const int kMaxCount = 50 * 400; // 50 * 400 * 2G = 40T
const size_t kMaxBundleSize = 2147483648; // 2G

MetaWriter::~MetaWriter() {
  if (-1 != id_) {
    char filename[1024];
    sprintf(filename, kLockNameFormat, id_);
    FileLock::Unlock(filename);
  }
}

int MetaWriter::Write(const char* buf, int size
  , const char *userdata, size_t userdata_size) {
  ASSERT(!filename_.empty());
  ASSERT(id_ >= 0);
  return Writer::WriteImpl(filename_.c_str()
    , offset(), length()
    , url(), buf, size
    , userdata, userdata_size);
}

Writer * MetaAllocator::Allocate(const char* prefix, int length) {
  std::string filename;
  char lockfile[1024];
  int id = 0;
  size_t filesize = 0;
  struct stat st;
  bool loop_once = false;

  // TODO: define root dir
  // TODO: use once
  if (0 !=stat("/mnt/mfs/.lock", &st)) {
    base::mkdirs("/mnt/mfs/.lock");
  }

  std::string date(prefix);
#if defined(OS_WIN)
  date += '\\';
#elif defined(OS_LINUX)
  date += '/';
#endif
  date += DateString();

  id = last_id_;
  do {
    // 1 最小代价找到一个可用文件
    // 2 try lock
    // 3 stat again

    // 1
    filename = BundleFilename(id, date.c_str());
    int nret = stat(filename.c_str(), &st);
    
    // 如果文件不存在，或者文件长度**未**超过限制
    if (-1 == nret || (Align1K(st.st_size) + length + kFileHeaderSize)< kMaxBundleSize) {
      // 2 
      sprintf(lockfile, kLockNameFormat, id);
      bool locked = FileLock::Lock(lockfile);
      if (locked) {
        // 3 再次 stat 很有必要，因为文件长度会在之后使用到
        nret = stat(filename.c_str(), &st);
        if (-1 == nret) {
          std::string dir = base::Dirname(filename);
          base::mkdirs(dir.c_str());

          // 创建文件
          bool f = CreateBundle(filename.c_str());
          ASSERT(f);
          if (f) {
            filesize = kBundleHeaderSize;
            break;
          }
        }

        if ((Align1K(st.st_size) + length + kFileHeaderSize) < kMaxBundleSize) {
          filesize = st.st_size;
          break;
        }

        // god, 居然没有成功
        FileLock::Unlock(lockfile);
      }
    }

    id ++;

    if (id >= kMaxCount) {
      if (loop_once) {
        // ASSERT(false);
        // LOG();
        // TODO: 发生什么事情了
      }
      id = last_expose_;
      loop_once = true;
    }
  } while (true);

  last_id_ = id + 1;

  DCHECK(filesize >= kBundleHeaderSize);
  Writer* pw = new MetaWriter(id, filename, filesize, length);
  return pw;
}

void MetaAllocator::Return(Writer * w) {
  delete w;
}

BundleAllocator* DefaultAllocator() {
  static MetaAllocator ma_;
  return &ma_;
}

}
