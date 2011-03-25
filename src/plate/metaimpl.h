#ifndef PLATE_METAIMPL_H__
#define PLATE_METAIMPL_H__

#include <stdio.h>
#include <string.h>
#include <boost/scoped_array.hpp>

#include "plate/itf.h"
#include "plate/tenandsixty.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace plate {

class MetaWriter : public Writer {
public:
  MetaWriter(int id, size_t offset, size_t length) 
    : id_ (id), offset_(offset), length_(length)
  {}
  ~MetaWriter();

  bool SetPrefix(const char* prefix, char type, const char* postfix = "jpg") {
    // 文件长度按 1K 取整
    if (id_ < 0 || offset_ < 0 || length_ <= 0)
      return false;

    int offset = Align1K(offset_);
    std::string index_tmp, offset_tmp, length_tmp;
    index_tmp = ToSixty(id_);
    offset_tmp = ToSixty(offset);
    length_tmp = ToSixty(length_);

    char buf[1024];
    snprintf(buf, sizeof(buf), "%s/%s/%c_%s_%s.%s", prefix
      , index_tmp.c_str()
      , type
      , offset_tmp.c_str()
      , length_tmp.c_str()
      , postfix);


    uint32 h = Writer::Hash(buf);
    snprintf(buf, sizeof(buf), "%s/%s/%c_%s_%s_%s.%s", prefix
      , index_tmp.c_str()
      , type
      , offset_tmp.c_str()
      , length_tmp.c_str()
      , ToSixty(h).c_str()
      , postfix);
    url_ = buf;

    return true;
  }

  const char* url() const {
    return url_.c_str(); // bad
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

private:
  int id_;
  size_t offset_;
  size_t length_;

  std::string url_;
};

struct MetaAllocator : public BundleAllocator {
  Writer * Allocate(int length);
  void Return(Writer * w);
};

}
#endif // PLATE_METAIMPL_H__
