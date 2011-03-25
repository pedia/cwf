#ifndef CELL_MFSCLIENT_FUSEIMPL_H__
#define CELL_MFSCLIENT_FUSEIMPL_H__

#ifdef WIN32
#define snprintf _snprintf
#endif

#include <boost/scoped_array.hpp>

#include "mfsclient/itf.h"
#include "mfsclient/tenandsixty.h"
#include "mfsclient/bundle.h"

#include "mfsclient/bundle_adapter.h"

namespace mfs {

class FuseWriter : public Writer {
public:
  FuseWriter(const Bundle& b) : bundle_(b) {}

  bool SetPrefix(const char* prefix, char type, const char* postfix = "jpg") {
    // 进行校验判断
    if (bundle_.index < 0 || bundle_.offset < 0 || bundle_.length <= 0)
      return false;

    std::string index_tmp, offset_tmp, length_tmp;
    index_tmp = ToSixty(bundle_.index);
    offset_tmp = ToSixty(bundle_.offset);
    length_tmp = ToSixty(bundle_.length);

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
      , length_tmp.c_str() // url只有文件内容的长度
      , ToSixty(h).c_str()
      , postfix);
    url_ = buf;

    return true;
  }

  const char* url() const {
    return url_.c_str(); // bad
  }

  uint32 index() const {
    return bundle_.index;
  }

  uint32 offset() const {
    return bundle_.offset;
  }

  uint32 length() const {
    return bundle_.length;
  }

private:
  Bundle bundle_;
  std::string url_;
};


class FuseBundleAllocator : public BundleAllocator {
public:
  FuseBundleAllocator() {
    prx_ = CreateBundleAdapter();
  }

  virtual Writer * Allocate(int length) {
    Bundle b = prx_->Request(length);
    return (new FuseWriter(b));
  }
  virtual void Return(Writer * p) {
    delete p;
  }
  virtual ~FuseBundleAllocator() {};

private:
  BundleCenterPrx prx_;
};

}
#endif // CELL_MFSCLIENT_FUSEIMPL_H__
