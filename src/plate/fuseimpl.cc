#include "mfsclient/fuseimpl.h"

namespace mfs {

BundleAllocator* DefaultAllocator() {
  static FuseBundleAllocator ba_;
  return &ba_;
}

}
