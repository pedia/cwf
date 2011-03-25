#include "plate/itf.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <boost/tokenizer.hpp>
#include <boost/scoped_array.hpp>

#include "base3/hash.h"
#include "base3/pathops.h"
#include "base3/mkdirs.h"
#include "base3/common.h"
#include "plate/tenandsixty.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace plate {

const char* kRoot = "/mnt/mfs";

uint32 Writer::Hash(const char* text, int text_length) {
  if (0 == text_length)
    text_length = strlen(text);
  return base::MurmurHash2(text, text_length, 0);
}

int Writer::WriteImpl(int index, int offset, int length
    , const std::string& url, const char* buf, int size
    , const char *userdata, size_t userdata_size) {
  // 进行校验判断
  if (index < 0 || offset < 0 || length <= 0)
    return 0;
  if (buf == NULL || size <= 0 || size != length)
    return 0;

  // 尽可能的减少远程通讯，浪费一些内存
  int padding = Align1K(offset) - offset;

  boost::scoped_array<char> huge(new char[padding + kHeaderSize + size]);
  memset(huge.get(), 0, padding + kHeaderSize + size);
  // 文件头处理
  {
    FileHeader* head = (FileHeader *)(huge.get() + padding);
    head->magic = FileHeader::kMAGIC_CODE;
    head->length = size + kHeaderSize;
    head->version = FileHeader::kVERSION;
    head->flag = FileHeader::kFILE_NORMAL;
    memcpy(head->url, url.c_str(), url.size());

    if (userdata_size && userdata_size <= ARRAY_SIZE(head->userdata))
      memcpy(head->userdata, userdata, userdata_size);
  }

  memcpy(huge.get() + padding + kHeaderSize, buf, size);

  char filepath[128] = {0};
  // 此处对index要进行取余解析
  BundleFilePath(filepath, 128, index);

  std::string dir = base::Dirname(filepath);
  struct stat st;
  if (0 != stat(dir.c_str(), &st)) {
    base::mkdirs(dir.c_str());
  }

  FILE *fp = fopen(filepath,"r+b");
  if (fp == NULL)
    return 0;
  if (fseek(fp, offset, 0) == -1)
    return 0;
  size_t written = fwrite(huge.get(), sizeof(char), padding + kHeaderSize + size, fp);
  ASSERT(padding + kHeaderSize + size == written);

  ASSERT(ftell(fp) == offset + padding + kHeaderSize + size);

  fclose(fp);
  return size;
}

bool Reader::Extract(const char* url, std::string *prefix
    , std::string *filename, char* type, uint32 *offset
    , uint32 *length, uint32 *hash) {
  if (url == NULL)
    return false;

  std::string str(url);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("/_.");
  tokenizer tokens(str, sep);

  // url has the fixed role, so take over the url below
  uint32 hash_tmp;
  std::string prefix_tmp, filename_tmp, type_tmp, offset_tmp, length_tmp, postfix_tmp;
  int i = 0;
  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter) {
    if (i == 0) prefix_tmp = *tok_iter + "/";
    else if (i == 1) prefix_tmp += *tok_iter + "/";
    else if (i == 2) prefix_tmp += *tok_iter;
    else if (i == 3) filename_tmp = *tok_iter;
    else if (i == 4) type_tmp = *tok_iter;
    else if (i == 5) offset_tmp = *tok_iter;
    else if (i == 6) length_tmp = *tok_iter;
    else if (i == 7) hash_tmp = FromSixty(*tok_iter);
    else if (i == 8) postfix_tmp = *tok_iter;
    ++i;
  }

  if (i != 9)
    return false;

  if (prefix) *prefix = prefix_tmp;

  if (filename) {
    char file[128] = {0};
    BundleFilePath(file, 128, FromSixty(filename_tmp));
    *filename = file;
  }

  if (type) *type = type_tmp[0];
  if (offset) *offset = FromSixty(offset_tmp);
  if (length) *length = FromSixty(length_tmp);
  if (hash) *hash = hash_tmp;

  // weather hash is right?
  char buf[1024] = {0};
  snprintf(buf, sizeof(buf), "%s/%s/%c_%s_%s.%s", prefix_tmp.c_str()
      , filename_tmp.c_str()
      , type_tmp[0]
      , offset_tmp.c_str()
      , length_tmp.c_str()
      , postfix_tmp.c_str());

  return Writer::Hash(buf) == hash_tmp;
}

struct AutoFile {
  AutoFile(FILE* fp = 0) : fp_(fp) {}
  ~AutoFile() {
    if (fp_)
      fclose(fp_);
  }
  
  FILE* get() const {
    return fp_;
  }

  FILE* fp_;
};

static int ReadTheFile(const char* posix_file, size_t offset, char * huge, unsigned int huge_size) {
  AutoFile fp(fopen(posix_file, "rb"));
  if (!fp.get())
    return 0;

  if (fseek(fp.get(), offset, 0) == -1)
    return 0;

  size_t readed = fread(huge, sizeof(char), huge_size, fp.get());
  if (readed >= kHeaderSize) {
    FileHeader * fh = (FileHeader *)huge;
    if (fh->magic != FileHeader::kMAGIC_CODE)
      return -1;

    if (fh->flag & FileHeader::kFILE_MARK_DELETE)
      return -1;

    if (fh->flag & FileHeader::kFILE_NORMAL) {
      if (fh->length != huge_size)
        return -1;

      if (readed == huge_size) {
        // memcpy(buffer, huge + kHeaderSize, huge_size - kHeaderSize);
        return huge_size - kHeaderSize;
      }

      // 只读了部分出来了?
      return -2;
    }

    // 
    if (fh->flag & FileHeader::kFILE_REDIRECT) {
      const char* url = huge + kHeaderSize;
      std::string filename;
      uint32 offset = 0, length = 0;

      if (Reader::Extract(url, NULL, &filename, NULL, &offset, &length, NULL) == false)
        return -1;

      return ReadTheFile(filename.c_str(), offset, huge, huge_size);
    }
  }

  return -2;
}

// 如果用户不先调用 Extract, 直接给定一个尽可能大的buffer，也要正确返回
// 虽然没有什么价值，但是也没有坏处
int Reader::Read(const char* url, char *buffer, int buffer_size) {
  std::string filename;
  uint32 offset = 0, file_size = 0;

  if (Extract(url, NULL, &filename, NULL, &offset, &file_size, NULL) == false)
    return -1;

  if (buffer == 0 || buffer_size == 0 || buffer_size < file_size)
    return file_size;

  // 先读 sizeof(FileHeader) + file_size
  // 检查是否被删除了，文件长度是否正确
  // 是否是跳转

  boost::scoped_array<char> huge(new char[kHeaderSize + file_size]);
  if (!huge)
    return 0;

  int readed = ReadTheFile(filename.c_str(), offset, huge.get(), kHeaderSize + file_size);
  if (readed == file_size) {
    memcpy(buffer, huge.get() + kHeaderSize, file_size);
  }
  return readed; 
}

bool CreateBundle(const char* filepath) {
  FILE* fp = fopen(filepath, "w+b");
  ASSERT(fp);
  if (!fp)
    return false;
  
  char now_str[100];
  {
    time_t t = time(0);
    struct tm *tmp = localtime(&t);
    strftime(now_str, 100, "%Y-%m-%d %H:%M:%S", tmp);
  }

  const int BundleHeaderSize = 4096;
  boost::scoped_array<char> huge(new char[BundleHeaderSize]);
  memset(huge.get(), 0, BundleHeaderSize);
  snprintf(huge.get(), BundleHeaderSize, "opi-corp.com file store\n"
      "1.0\n"
      "%s\n", now_str);
  fwrite(huge.get(), sizeof(char), BundleHeaderSize, fp);
  fclose(fp);
  return true;
}

const char* kBundleNameFormat = "%s/p/%02d/%08d";

// 从 bundle index 得到文件全路径

// 总共 40T
// 40T / 2G = 20K 个文件
// 保持一个原则，每个目录下文件数不超过 400
// 50 * 400 = 20000
// TODO: 好像和之前预想的有差异，更好的实现
int BundleFilePath(char* name, int name_size, unsigned int i) {
#ifdef OS_LINUX
  int ret = snprintf(name, name_size, kBundleNameFormat, kRoot, i % 50, i);
#elif defined(WIN32)
  int ret = snprintf(name, name_size, "f:\\mnt\\plate\\p\\%02x\\%08x", i % 50, i);
#endif
  ASSERT(ret > 0); // TODO: use DCHECK
  return ret;
}

}
