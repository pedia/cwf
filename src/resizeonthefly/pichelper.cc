#include "resizeonthefly/pichelper.h"

#include "base3/logging.h"

#include "magick/resize.h"
#include "magick/blob.h"
#include "magick/magick.h"
#include "magick/transform.h" // Sharpen
#include "magick/profile.h"
#include "magick/constitute.h" // Read/Write Image
#include "magick/attribute.h" // Quality attribute

namespace rof {

SizeCheckResult SizeCheck(const Size & img_size, const Size & min_size
               , const RatioPair * const ratio // = NULL
               ) {
  // 图片尺寸不能小于
  if (img_size.width < min_size.width || img_size.height < min_size.height) {
    return kTOO_SMALL;
  }

  if (!ratio)
    return kNORMAL;

  // 宽/高 < 1/3的图片将被过滤
  if ((double)img_size.width / img_size.height <= ratio->first) {
    return kRATIO_TOO_SMALL;
  }

#if 0
  // 尺寸策略
  if ((double)img_size.width / img_size.height > 1) {
    // W/H>1  Max width=600
    target.cx = std::min(600, img_size.width);
    target.cy = (double)(target.cx * img_size.height) / img_size.width;
  } else {
    // W/H<1  Max width=600 && Max height=900
    target.cy = std::min(900, img_size.height);
    target.cx = (double)(target.cy * img_size.width) / img_size.height;

    if (target.cx > 600) {
      target.cx = std::min(600, img_size.width);
      target.cy = (double)(target.cx * img_size.height) / img_size.width;
    }
  }
#endif
  return kNORMAL;
}

bool AutoImage::Init(const char* buf, int size) {
  ExceptionInfo ec;
  GetExceptionInfo(&ec);

  ImageInfo * imginfo = CloneImageInfo(0);
  if (!imginfo)
    return false;

  // 从来都不存在 struct _BlobInfo，强制转换
  imginfo->blob = (struct _BlobInfo *)(void *)buf;
  imginfo->length = size;

#if 0
  // 在 ReadImage 里都有设置，没有必要调用
  SetImageInfo(imginfo, SETMAGICK_READ, &ec);

  const MagickInfo * magick_info = GetMagickInfo(imginfo->magick, &ec);

  if (magick_info == (const MagickInfo *) NULL) {
    DestroyImageInfo(imginfo);
    return false;
  }
#endif

  Image * img = ReadImage(imginfo, &ec);
  if (!img) {
    LOG(ERROR) << "ReadImage failed reason: " << ec.reason << " in "
      << ec.module << " line:" << ec.line;
    DestroyImageInfo(imginfo);
    return false;
  }

  DestroyImageInfo(imginfo);

  Set(img);
  return true;
}

bool AutoImage::Resize(int max_width, int quality) {
  if (img_->columns == 0)
    return false;

  if (img_->columns <= max_width)
    return true; // TODO: 如果高度需要缩小呢

  ExceptionInfo ec;
  GetExceptionInfo(&ec);

  // TOOD: use the SizeCheck
  int h = (double)img_->rows * max_width / img_->columns;

  // blur = 0.8 for sharpen
  Image * newimg = ResizeImage(img_, max_width, h, img_->filter, 0.8, &ec);
  Set(newimg);
  return true;
}

bool AutoImage::WriteToBlob(char ** bufptr, int * size) {
  ImageInfo * imginfo = CloneImageInfo(0);
  if (!imginfo)
    return false;

  ExceptionInfo ec;
  GetExceptionInfo(&ec);

#if 1
  // 保持质量不变
  const ImageAttribute * attr = GetImageAttribute(img_, "JPEG-Quality");
  if (attr) {
    int q = atoi(attr->value);
    if (q)
      imginfo->quality = q;
  }

  // EXIF 信息
  // TODO: 有内存泄漏
//   imginfo->attributes = CloneImage(img_, 0, 0, 1, &ec);
//   if (imginfo->attributes) {
//     MagickPassFail fail = CloneImageAttributes(imginfo->attributes, img_);
//    // TODO: check fail
//   }
#else
  // jpeg:preserve-settings操作复杂, 貌似没有太大作用
  AddDefinitions(imginfo, "jpeg:preserve-settings=1", &ec);
#endif

  // TODO:
  imginfo->interlace == LineInterlace;
  // TODO: x_resolution

  size_t length;
  void* p= ImageToBlob(imginfo, img_, &length, &ec);
  if (p && bufptr) {
    *bufptr = (char*)p;
    if (size)
      *size = length;

    DestroyImageInfo(imginfo);
    return true;
  }

  if (p)
    MagickFree(p);

  DestroyImageInfo(imginfo);
  return p != NULL;
}

void AutoImage::Free(void * buf) {
  MagickFree(buf);
}

bool AutoImage::Crop(int width, int height) {
  if (img_->columns <= width && img_->rows <= height)
    return true;

  ExceptionInfo ec;
  GetExceptionInfo(&ec);

  RectangleInfo rc;
  rc.x = (img_->columns - width) / 2;
  rc.y = (img_->rows - height) / 2;
  rc.width = width;
  rc.height = height;
  Image * newimg = CropImage(img_, &rc, &ec);
  Set(newimg);
  return true;
}

bool AddWaterMark(Image* img, Image* warter) {
  return false;
}

// size check
// remove image.comment
// transfer cmyk to rgb if need
// resize
// watermark if need // util/watermark/Page.jpg
// // save interlaceType(Magick::PlaneInterlace
bool Process(Image * img
             , const Size & min_size
             , const RatioPair * const ratio /*= NULL*/
             , std::string * error_detail /*= NULL*/) {
  Size size(img->columns, img->rows);

  // 1
  if (kNORMAL != SizeCheck(size, min_size, ratio)) {
    // 
    return false;
  }

  // 2
  // 去掉所有的 profile 信息, 该信息尺寸巨大，在所有 exif 中占得最多
  // TODO: throwImageException
  // img->profile("*", Magick::Blob());
  int ret = DeleteImageProfile(img, "*");
  // check ret == 1

  // 3
  // ret = TransformRGBImage(img, RGBColorspace);
  // check ret == 1

  // 4
  // if crop, call CropImage
  // else
  //   calculate regular size
  //   TODO: 如果尺寸不变，会导致对象拷贝
  //   ResizeImage

  // 5 watermark

  return false;
}

void InitMagickLib() {
  char* argv = {0};
  InitializeMagick(argv);
}

void DestroyMagickLib() {
  DestroyMagick();
}

}
