#include "plate/metaimpl.h"

#include <errno.h>

#include "base3/pathops.h"
#include "base3/mkdirs.h"
#include "base3/logging.h"
#include "base3/atomicops.h"

#include "plate/fslock.h"

namespace plate {

typedef base::subtle::Atomic32 AtomicCount;

inline AtomicCount AtomicCountInc(volatile AtomicCount *ptr,
     AtomicCount increment = 1) {
  return base::subtle::NoBarrier_AtomicIncrement(ptr, increment);
}

inline void AtomicCountStore(volatile AtomicCount *ptr,
     AtomicCount value) {
  base::subtle::NoBarrier_Store(ptr, value);
}

inline AtomicCount AtomicCountLoad(volatile AtomicCount *ptr) {
  return base::subtle::NoBarrier_Load(ptr);
}

// TODO: use kRoot
static const char* kLockNameFormat = "/mnt/mfs/.lock/%08x";

const int kMaxCount = 50 * 400; // 50 * 400 * 2G = 40T
const size_t kMaxBundleSize = 2 * 1024 * 1024 * 1024; // 2G

static volatile AtomicCount last_id_ = 0;

// 每次写入一定数目的 Bundle 内
// [0, kStageSize)
// [kStageSize, kStageSize*2)
// ...
// [kStageSize * current_stage_, kStageSize*(current_stage_+1))
static volatile AtomicCount current_stage_ = 0;
const int kStageSize = 400;

int CurrentMax() {
  return kStageSize * (AtomicCountLoad(&current_stage_) + 1);
}

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

  int expose_count = 0; // 文件长度超过的个数

  id = AtomicCountLoad(&last_id_);
  int prev_id = id;
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
    expose_count ++;

    if (id >= CurrentMax() ) {
      if (expose_count >= kStageSize) {
        int current_stage = AtomicCountInc(&current_stage_);
        if (current_stage > kMaxCount/kStageSize) {
          AtomicCountStore(&current_stage_, 0);
          LOG(INFO) << "All Stage exposed expose: " << expose_count
            << " current stage:" << current_stage_;
          id = 0;
          continue;
        }
      } else {
        int oldid = id;
        id = kStageSize * AtomicCountLoad(&current_stage_);
        LOG(INFO) << "Next Stage expose: " << expose_count
          << " id: "
          << prev_id << " > "
          << oldid << " > " 
          << id;
        continue;
      }
    }
  } while (true);

  AtomicCountStore(&last_id_, id);
  VLOG(1) << "Next id: " 
    << prev_id << " > "
    << id
    << " expose: " << expose_count;

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
