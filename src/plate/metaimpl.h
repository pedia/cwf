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
  MetaWriter(int id, const std::string & filename, size_t offset, size_t length) 
    : Writer(id, offset, length),
      filename_(filename)
  {}
  ~MetaWriter();

  int Write(const char* buf, int size
    , const char *userdata = 0, size_t userdata_size = 0);

private:
  
  std::string filename_;
};

struct MetaAllocator : public BundleAllocator {
  MetaAllocator() : last_expose_(0) {}
  Writer * Allocate(const char* prefix, int length);
  void Return(Writer * w);
private:
  int last_expose_; // 容量超过了最大值的最大 id
};

}
#endif // PLATE_METAIMPL_H__
