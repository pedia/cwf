#include "plate/metaimpl.h"

#include "base3/pathops.h"
#include "base3/mkdirs.h"

#include "plate/fslock.h"

namespace plate {

// TODO: use kRoot
static const char* kLockNameFormat = "/mnt/mfs/.lock/%08x";

static volatile int last_id_ = 0;
const int kMaxCount = 200 * 1024; // 400T/2G
const size_t kMaxBundleSize = 2147483648; // 2G

MetaWriter::~MetaWriter() {
  if (-1 != id_) {
    char filename[1024];
    sprintf(filename, kLockNameFormat, id_);
    FileLock::Unlock(filename);
  }
}

Writer * MetaAllocator::Allocate(int length) {
  char filename[1024];
  int id = 0;
  size_t filesize = 0;
  struct stat st;
  bool loop_once = false;

  // TODO: define root dir
  if (0 !=stat("/mnt/mfs/.lock", &st)) {
    base::mkdirs("/mnt/mfs/.lock");
  }

  id = last_id_;
  do {
    // 1 最小代价找到一个可用文件
    // 2 lock
    // 3 stat again

    // 1
    BundleFilePath(filename, sizeof(filename), id);
    int nret = stat(filename, &st);
    if (-1 == nret || (Align1K(st.st_size) + length + kHeaderSize)< kMaxBundleSize) {
      // 2 
      sprintf(filename, kLockNameFormat, id);
      bool locked = FileLock::Lock(filename);
      if (locked) {
        // 3
        BundleFilePath(filename, sizeof(filename), id);
        nret = stat(filename, &st);
        if (-1 == nret) {
          std::string dir = base::Dirname(filename);
          base::mkdirs(dir.c_str());

//           std::cout << "Create: " << filename 
//             << " Dir: " << dir << "\n";

          // 创建文件
          bool f = CreateBundle(filename);
          ASSERT(f);
          if (f) {
            filesize = 4096;
            break;
          }
        }

        if ((Align1K(st.st_size) + length + kHeaderSize) < kMaxBundleSize) {
          filesize = st.st_size;
          break;
        }

        // god, 居然没有成功
        FileLock::Unlock(filename);
      }
    }

    id ++;

    if (id >= kMaxCount) {
      if (loop_once) {
        // ASSERT(false);
        // LOG();
        // TODO: 发生什么事情了
      }
      id = 0;
      loop_once = true;
    }
  } while (true);

  last_id_ = id;

  // std::cout << "Call Writer, offset: " << filesize << "\n";
  return new MetaWriter(id, filesize, length);
}

void MetaAllocator::Return(Writer * w) {
  delete w;
}

BundleAllocator* DefaultAllocator() {
  static MetaAllocator ma_;
  return &ma_;
}

}
