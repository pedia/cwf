#ifndef PLATE_INTERFACE_H__
#define PLATE_INTERFACE_H__

// 此文件尽可能保持独立

#include <string>
#include "base3/basictypes.h"

namespace plate {

struct FileHeader {
  enum {
    kMAGIC_CODE = 0xE4E4E4E4,
    kVERSION = 1,
    kFILE_NORMAL = 1,
    kFILE_MARK_DELETE = 2,
    kFILE_REDIRECT = 4,
  };

  uint32 magic;
  uint32 length;
  uint32 version;
  uint32 flag;
  char url[256];
  char userdata[512 - 256 - 4*4];
};

enum {
  kFileHeaderSize = sizeof(FileHeader),
  kBundleHeaderSize = 4096
};

// 应用(上传程序)如何使用:
// Writer* aw = BundleAllocator::Allocate(length);
// aw.SetUrlDetail("fmn04", "20110331", 'M', "gif");
// aw.Write(buf, size)
// BundleAllocator::Return(aw);

// file: fmn04/20110331/ [index/50] / [index%50]
// url: fmn04/20110331/1240/ index /M_xxx_xxx_xxx.gif

struct Writer {
  // url 规则**必须**是  
  // host/year month day/hour minute/bundle/Type_offset_length_hash.[jpg|gif]
  // fmn04/20010625/1035/BDSNtDAoQja/Type_12NtDA_3Nt_a3NtDA.jpg
  // -----|<--prefix  ->| bundle id |<-

  // 文件实际存在
  // /mnt/mfs/20100625/bundle
  // 的一个大文件里，长度通常限制在 2G 左右

  // fmn04 的目的是和之前的 url 规则兼容
  // date = "20010625/1035" 给 CDN 优化、管理使用
  // bundle, offset, length, hash 整数全部转为 60 进制是为了缩短长度，实现过程中还要加密(抑或)一下
  // Hash 的目的是防止前后部分可变化，验证 url 合法性
  // hash = func('fmn04/20010625/1035/BDSNtDAoQja/T_12NtDA_3Nt.jpg')
  // type = L(large) M(main) S(small) T(tiny)
  bool SetUrlDetail(const char* prefix, const char* date = 0, char type='R', const char* postfix = "jpg");

  static uint32 Hash(const char* text, int text_length = 0);

  // Write 直接操纵 fuse 文件，大致实现为
  // open()
  // write[magic code, total length, url]
  // write(buf)
  static int WriteImpl(const std::string& filename
    , int offset, int length
    , const std::string& url, const char* buf, int size
    , const char *userdata = 0, size_t userdata_size = 0);

  virtual int Write(const char* buf, int size
    , const char *userdata = 0, size_t userdata_size = 0) = 0;

  // close()
  // 由于可能是远程调用，一次写完代价稍微小一些

  // TODO: 提供可以多次写入的接口

  const std::string & url() const {
    return url_;
  }

  uint32 index() const {
    return id_;
  }

  uint32 offset() const {
    return offset_;
  }

  uint32 length() const {
    return length_;
  }

  Writer(int id, size_t offset, size_t length) 
    : id_ (id), offset_(offset), length_(length)
  {}
  virtual ~Writer() {}

protected:
  int id_;
  size_t offset_;
  size_t length_;

  std::string url_;
};

struct BundleAllocator {
  virtual Writer * Allocate(const char* prefix, int length) = 0;
  virtual void Return(Writer *) = 0;
  virtual ~BundleAllocator() {};
};

struct Reader {
  static bool Extract(const char* url, std::string *path = 0
    , std::string *filename = 0, char* type = 0, uint32 *offset = 0
    , uint32 *length = 0, uint32 *hash = 0);

  // 如果 buffer = 0, 返回 Extract 后的 length
  // 1 url => file, offset, length, hash
  // 2 hash(url) == hash?
  // 3 dfs's open(file)
  // 4 seek
  // 5 read

  static int Read(const char* url, char *buffer, int size);
};

BundleAllocator* DefaultAllocator();

// 向上 1k 取整
template<typename T>
inline T Align1K(T v) {
  T a = v % 1024;
  return a ? (v + 1024 - a) : v;
}

bool CreateBundle(const char* filepath);

std::string DateString();
std::string TimeString();

// 1 url => file
// 2 index + today => file
std::string BundleFilename(unsigned int bundle_index, const char* prefix = 0);

}
#endif // PLATE_INTERFACE_H__
