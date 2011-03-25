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
  kHeaderSize = sizeof(FileHeader)
};

// 应用(上传程序)如何使用:
// Writer* aw = BundleAllocator::Allocate(length);
// aw.SetPrefix("fmn04");
// aw.Write(photo content)
// BundleAllocator::Return(aw);

struct Writer {
  // url 规则**必须**是  
  // host/year month day/hour minute/bundle/Type_offset_length_hash.[jpg|gif]
  // fmn04/20010625/1035/BDSNtDAoQja/Type_12NtDA_3Nt_a3NtDA.jpg
  // -------prefix------|

  // 文件实际存在
  // /mnt/mfs/20100625/bundle
  // 的一个大文件里，长度通常限制在 1G 左右

  // fmn04 的目的是和之前的 url 规则兼容
  // 20010625/1035 给 CDN 优化、管理使用
  // bundle, offset, length, hash 整数全部转为 60 进制是为了缩短长度，实现过程中还要加密(抑或)一下
  // Hash 的目的是防止前后部分可变化，验证 url 合法性
  // hash = func('fmn04/20010625/1035/BDSNtDAoQja/T_12NtDA_3Nt.jpg')
  // type = L(large) M(main) S(small) T(tiny)
  virtual bool SetPrefix(const char* prefix, char type='R', const char* postfix = "jpg") = 0;

  // Write 直接操纵 fuse 文件，大致实现为
  // open()
  // write[magic code, total length, url]
  // write(buf)

  // close()
  // 由于可能是远程调用，一次写完代价稍微小一些

  // TODO: 提供可以多次写入的接口
  
  virtual const char* url() const = 0;
  virtual uint32 index() const = 0;
  virtual uint32 offset() const = 0;
  virtual uint32 length() const = 0;

  static uint32 Hash(const char* text, int text_length = 0);

  static int WriteImpl(int index, int offset, int length
    , const std::string& url, const char* buf, int size
    , const char *userdata = 0, size_t userdata_size = 0);

  // TODO:
  // static BuildUrl(const char* prefix, const char* int bundle_index);
  virtual ~Writer() {};
};

struct BundleAllocator {
  virtual Writer * Allocate(int length) = 0;
  virtual void Return(Writer *) = 0;
  virtual ~BundleAllocator() {};
};

struct Reader {
  static bool Extract(const char* url, std::string *prefix
    , std::string *filename, char* type, uint32 *offset
    , uint32 *length, uint32 *hash);

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

int BundleFilePath(char* name, int name_size, unsigned int bundle_index);

}
#endif // PLATE_INTERFACE_H__
