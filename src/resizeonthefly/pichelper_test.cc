#include <gtest/gtest.h>

#include <string.h>

#include <fstream>
#include <vector>

#include "resizeonthefly/pichelper.h"

#include "google/profiler.h"

#include "magick/magick.h"
#include "magick/constitute.h"
#include "magick/resize.h"
#include "magick/effect.h"

using namespace rof;

#if 0
TEST(PicHelper, SizeCheck) {
  Size target;
  EXPECT_TRUE(SizeCheck(Size(1, 1), target));
  EXPECT_TRUE(SizeCheck(Size(10, 10), target));
  EXPECT_TRUE(SizeCheck(Size(1000, 1), target));
  EXPECT_TRUE(SizeCheck(Size(10000, 1), target));

  EXPECT_FALSE(SizeCheck(Size(1, 1000), target));
  EXPECT_FALSE(SizeCheck(Size(200, 1000), target));
  EXPECT_FALSE(SizeCheck(Size(300, 900), target));
  EXPECT_TRUE(SizeCheck(Size(301, 900), target));

  EXPECT_TRUE(SizeCheck(Size(1560, 900), target));
  EXPECT_EQ(target, Size(600, 346));

  EXPECT_TRUE(SizeCheck(Size(780, 900), target));
  EXPECT_EQ(target, Size(600, 692));

  EXPECT_TRUE(SizeCheck(Size(600, 1000), target));
  EXPECT_EQ(target, Size(540, 900));

  EXPECT_TRUE(SizeCheck(Size(500, 800), target));
  EXPECT_EQ(target, Size(500, 800));
}
#endif

class WithGraphMagick: public testing::Test {
protected:
  void SetUp() {
    char* argv = {0};
    InitializeMagick(argv);

    ProfilerStart("/tmp/withgm.perf");
  }
  void TearDown() {
    DestroyMagick();

    ProfilerStop();
  }
};

TEST_F(WithGraphMagick, Resize) {
  ExceptionInfo ec;
  GetExceptionInfo(&ec);

  ImageInfo * image_info = CloneImageInfo(NULL);
  strncpy(image_info->filename, "test.jpg", MaxTextExtent);

  Image * img = ReadImage(image_info, &ec);
  ASSERT_TRUE(img != NULL);

  Image * img_resized = ResizeImage(img, 360, 275, UndefinedFilter, 1.0, &ec);
  ASSERT_TRUE(img_resized != NULL);

  // http://redskiesatnight.com/2005/04/06/sharpening-using-image-magick/
  // http://www.petapixel.com/2009/05/19/sharpening-your-photos-like-flickr/
  Image * img_sharpen = SharpenImage(img_resized, 1.5, 1, &ec);

  ImageInfo * image_info_resized = CloneImageInfo(NULL);
  strncpy(image_info_resized->filename, "test_resized.jpg", MaxTextExtent);
  unsigned int ret = WriteImage(image_info_resized, img_resized);
  EXPECT_TRUE(ret);

  DestroyImage(img_sharpen);
  DestroyImage(img);
  DestroyImage(img_resized);

  DestroyImageInfo(image_info);

  DestroyMagick();
}

TEST_F(WithGraphMagick, Blob) {
  std::ifstream istem("test.jpg", std::ios::binary);
  istem.unsetf(std::ios::skipws);
  std::vector<char> data;
  std::copy(
    std::istream_iterator<char>(istem), std::istream_iterator<char>()
    , std::back_inserter(data)
    );

  AutoImage ai;
  bool f = ai.Init(&data[0],data.size());
  ASSERT_TRUE(f);

  char *p = 0;
  int size = 0;
  f = ai.WriteToBlob(&p, &size);
  EXPECT_TRUE(f);

  std::ofstream ostem("test_w.jpg", std::ios::binary);
  ostem.write((const char*)p, size);
}
