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

uint32 Writer::Hash(const char* text, int text_length) {
  if (0 == text_length)
    text_length = strlen(text);
  return base::MurmurHash2(text, text_length, 0);
}

bool Writer::SetUrlDetail(const char* prefix, const char* date, char type, const char* postfix) {
  if (index() < 0 || offset() <= 0 || length() <= 0)
    return false;

  // 文件长度按 1K 取整
  int padding_offset = Align1K(offset());
  std::string index_tmp, offset_tmp, length_tmp;
  index_tmp = ToSixty(index());
  offset_tmp = ToSixty(padding_offset);
  length_tmp = ToSixty(length());

  std::string date_str;
  if (!date) {
    date_str = DateString();
    date = date_str.c_str();
  }
  std::string time_str = TimeString();

  char buf[1024];
  snprintf(buf, sizeof(buf), "%s/%s/%s/%s/%c_%s_%s.%s", prefix
    , date
    , time_str.c_str()
    , index_tmp.c_str()
    , type
    , offset_tmp.c_str()
    , length_tmp.c_str()
    , postfix);

  uint32 h = Writer::Hash(buf);
  snprintf(buf, sizeof(buf), "%s/%s/%s/%s/%c_%s_%s_%s.%s", prefix
    , date
    , time_str.c_str()
    , index_tmp.c_str()
    , type
    , offset_tmp.c_str()
    , length_tmp.c_str()
    , ToSixty(h).c_str()
    , postfix);

  url_ = buf;
  return true;
}

int Writer::WriteImpl(const std::string& filename
    , int offset, int length
    , const std::string& url, const char* buf, int size
    , const char *userdata, size_t userdata_size) {
  // 进行校验判断
  if (filename.empty() || offset <= 0)
    return 0;

  // 尽可能的减少远程通讯，浪费一些内存
  int padding = Align1K(offset) - offset;

  boost::scoped_array<char> huge(new char[padding + kFileHeaderSize + size]);
  memset(huge.get(), 0, padding + kFileHeaderSize + size);
  // 文件头处理
  {
    FileHeader* head = (FileHeader *)(huge.get() + padding);
    head->magic = FileHeader::kMAGIC_CODE;
    head->length = size + kFileHeaderSize;
    head->version = FileHeader::kVERSION;
    head->flag = FileHeader::kFILE_NORMAL;
    memcpy(head->url, url.c_str(), url.size());

    if (userdata_size && userdata_size <= ARRAY_SIZE(head->userdata))
      memcpy(head->userdata, userdata, userdata_size);
  }

  memcpy(huge.get() + padding + kFileHeaderSize, buf, size);

#if 0
  // 一般CreateBundle创建了目录，此处不需要
  std::string dir = base::Dirname(filepath);
  struct stat st;
  if (0 != stat(dir.c_str(), &st)) {
    base::mkdirs(dir.c_str());
  }
#endif

  FILE *fp = fopen(filename.c_str(), "r+b");
  if (fp == NULL)
    return 0;
  if (fseek(fp, offset, 0) == -1)
    return 0;
  size_t written = fwrite(huge.get(), sizeof(char), padding + kFileHeaderSize + size, fp);
  ASSERT(padding + kFileHeaderSize + size == written);

  ASSERT(ftell(fp) == offset + padding + kFileHeaderSize + size);

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
  std::string prefix_tmp, date_tmp, time_tmp, index_tmp, type_tmp, offset_tmp, length_tmp, postfix_tmp;
  int i = 0;
  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter) {
    if (i == 0) prefix_tmp = *tok_iter;
    else if (i == 1) date_tmp += *tok_iter;
    else if (i == 2) time_tmp += *tok_iter;
    else if (i == 3) index_tmp = *tok_iter;
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
    // prefix/date/ index / [] / []
    int id = FromSixty(index_tmp);
    char t[128];
    snprintf(t, 128, "%s/%s", prefix_tmp.c_str()
      , date_tmp.c_str());

    *filename = BundleFilename(id, t);

#if defined(OS_WIN)
    std::string::size_type pos = (*filename).find('/');
    while (pos != std::string::npos) {
      *filename = (*filename).replace(pos, 1, 1, '\\');
      pos = (*filename).find('/', pos + 1);
    }
#endif
  }

  if (type) *type = type_tmp[0];
  if (offset) *offset = FromSixty(offset_tmp);
  if (length) *length = FromSixty(length_tmp);
  if (hash) *hash = hash_tmp;

  // weather hash is right?
  char buf[1024] = {0};
  snprintf(buf, sizeof(buf), "%s/%s/%s/%s/%c_%s_%s.%s", prefix_tmp.c_str()
      , date_tmp.c_str()
      , time_tmp.c_str()
      , index_tmp.c_str()
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
  if (readed >= kFileHeaderSize) {
    FileHeader * fh = (FileHeader *)huge;
    if (fh->magic != FileHeader::kMAGIC_CODE)
      return -1;

    if (fh->flag & FileHeader::kFILE_MARK_DELETE)
      return -1;

    if (fh->flag & FileHeader::kFILE_NORMAL) {
      if (fh->length != huge_size)
        return -1;

      if (readed == huge_size) {
        // memcpy(buffer, huge + kFileHeaderSize, huge_size - kFileHeaderSize);
        return huge_size - kFileHeaderSize;
      }

      // 只读了部分出来了?
      return -2;
    }

    // 
    if (fh->flag & FileHeader::kFILE_REDIRECT) {
      const char* url = huge + kFileHeaderSize;
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

  boost::scoped_array<char> huge(new char[kFileHeaderSize + file_size]);
  if (!huge)
    return 0;

  int readed = ReadTheFile(filename.c_str(), offset, huge.get(), kFileHeaderSize + file_size);
  if (readed == file_size) {
    memcpy(buffer, huge.get() + kFileHeaderSize, file_size);
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

  boost::scoped_array<char> huge(new char[kBundleHeaderSize]);
  memset(huge.get(), 0, kBundleHeaderSize);
  snprintf(huge.get(), kBundleHeaderSize, "opi-corp.com file store\n"
      "1.0\n"
      "%s\n", now_str);
  fwrite(huge.get(), sizeof(char), kBundleHeaderSize, fp);
  fclose(fp);
  return true;
}

// used in file
std::string DateString() {
  char now_str[16];
  time_t t = time(0);
  struct tm *tmp = localtime(&t);

  strftime(now_str, 100, "%Y%m%d", tmp);
  return std::string(now_str);
}

// used in url
std::string TimeString() {
  char now_str[16];
  time_t t = time(0);
  struct tm *tmp = localtime(&t);

  strftime(now_str, 100, "%H%M", tmp);
  return std::string(now_str);
}

// 从 bundle index 得到文件全路径
// 保持一个原则，每个目录下文件数不超过 400
// 50 * 400 = 20000 * 2G = 40T
// TODO: 好像和之前预想的有差异，更好的实现

const char* kRoot = "/mnt/mfs";
const char* kPrefix = "p";
const char* kBundleNameFormat = "%s/%s/%s/%02x/%03x";
//   foo/20101231/ [index/50] / [index%400]

std::string BundleFilename(unsigned int bundle_index, const char* prefix) {
#ifdef OS_LINUX
  const int name_size = 1024;
  char name[name_size];

  if (!prefix) {
    int ret = snprintf(name, name_size, "%s/%s/%s/%02d/%04d", kRoot, kPrefix
      , DateString().c_str()
      , bundle_index % 50, bundle_index);
    ASSERT(ret > 0); // TODO: use DCHECK
  } else {
    int ret = snprintf(name, name_size, "%s/%s/%02d/%04d", kRoot, prefix
      , bundle_index % 50, bundle_index);
    ASSERT(ret > 0); // TODO: use DCHECK
  }
  
  return std::string(name);
#elif defined(OS_WIN)
  const int name_size = 1024;
  char name[name_size];

  if (!prefix) {

#define kRoot "f:\\mnt\\plate"

    int ret = snprintf(name, name_size, "%s\\%s\\%s\\%02d\\%04d", kRoot, kPrefix
      , DateString().c_str()
      , bundle_index % 50, bundle_index);
    ASSERT(ret > 0);
  } else {
    int ret = snprintf(name, name_size, "%s\\%s\\%02d\\%04d", kRoot, prefix
      , bundle_index % 50, bundle_index);
    ASSERT(ret > 0);
  }

  std::string a(name);
  // hack
  std::string::size_type pos = a.find('/');
  if (pos != -1)
    a = a.replace(pos, 1, 1, '\\');
  return a;
#endif  
}

}
