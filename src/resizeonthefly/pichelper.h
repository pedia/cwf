#ifndef RESIZEONTHEFLY_PIC_HELPER_H__
#define RESIZEONTHEFLY_PIC_HELPER_H__

#include <utility>
#include <string>

#include "magick/studio.h"

namespace rof {

enum SizeCheckResult {
  kNORMAL,
  kTOO_SMALL,
  kRATIO_TOO_SMALL,
  kRATIO_TOO_LARGE,
};

struct Size {
  int width, height;
  Size(int w_ = 0, int h_ = 0) : width(w_), height(h_) {}
};

inline bool operator==(const Size& s1, const Size &s2) {
  return s1.width == s2.width && s1.height == s2.height;
}

typedef std::pair<double, double> RatioPair;


SizeCheckResult SizeCheck(const Size & img_size
                          , const Size & min_size
                          , const RatioPair * const ratio = NULL
                          );

struct AutoImage {
  AutoImage(Image * img = 0) : img_(img) {}

  ~AutoImage() {
    if (img_)
      DestroyImage(img_);
  }

  bool Init(const char* buf, int size);

  bool Resize(int max_width, int quality);
  bool Crop(int width, int height);

  bool WriteToBlob(char ** bufptr, int * size);

  void Free(void * buf);

  void Set(Image* new_img) {
    if (img_)
      DestroyImage(img_);

    img_ = new_img;
  }
  Image* img_;
};

bool AddWaterMark(Image* img, Image* warter);

bool Process(Image * img
             , const Size & min_size
             , const RatioPair * const ratio = NULL
             , std::string * error_detail = NULL);

// Q: why wrap?
// A: much defines, includes
void InitMagickLib();
void DestroyMagickLib();
}
#endif // RESIZEONTHEFLY_PIC_HELPER_H__
