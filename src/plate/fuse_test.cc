#include <gtest/gtest.h>
#include <sys/stat.h>
#include "base3/mkdirs.h"
#include "base3/pathops.h"
#include "plate/tenandsixty.h"
#include "plate/fuseimpl.h"

using namespace plate;

const int64 kFileMaxLength = (int64)1024 * 1024 * 10;

TEST(FuseWriter, write) {
  const char* text = "05208072545253756145";
  const int content_length = strlen(text);
  Bundle t;
  t.offset = 0;
  t.index = 231; // 31/0000231
  t.length = content_length;
  std::string url;

  char file[128] = {0};
  BundleFilePath(file, 128, t.index);
  std::string filepath = xce::Dirname(file);
  // 建立测试文件夹和文件
  {
    if ((access(filepath.c_str(), 0)) == -1) {
      if (xce::mkdirs(filepath.c_str(), S_IRWXU | 
        S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
          std::cout << "NewNode mkdir err" << std::endl;
    }

    if(CreateBundle(file) != true)
    std::cout << "CreateBundle err" << std::endl;
  }

  {
    FuseWriter c(t);
    EXPECT_TRUE(c.SetPrefix("fmn04/20010625/1035", 'L'));
    url = c.url();

    EXPECT_EQ(content_length, plate::Writer::WriteImpl(
      c.index(), c.offset(), c.length(),
      c.url(), text, content_length));
  }

  {
    // 测试Reader的Read,一定要多一个字节，否则在strcmp判断时不能判断结尾
    const int32 size = content_length + 1;
    char buffer[size];
    memset(buffer, 0, size);

    EXPECT_TRUE(plate::Reader::Read(url.c_str(), buffer, size) == content_length);
    EXPECT_TRUE(strcmp(buffer, text) == 0);
  }

  // 删除临时文件夹和文件
  remove(file);
  rmdir(filepath.c_str());
  rmdir(xce::Dirname(filepath).c_str());

  // 错误情况的测试
  // 没有建立URL
  {
    FuseWriter c(t);
    EXPECT_EQ(0, plate::Writer::WriteImpl(
      c.index(), c.offset(), c.length(),
      c.url(), text, content_length));
  }

  // 写参数不正确
  {
    FuseWriter c(t);
    EXPECT_TRUE(c.SetPrefix("fmn04/20010625/1035", 'L'));

    EXPECT_EQ(0, plate::Writer::WriteImpl(
      c.index(), c.offset(), c.length(),
      c.url(), text, 0));

    EXPECT_EQ(0, plate::Writer::WriteImpl(
      c.index(), c.offset(), c.length(),
      c.url(), 0, content_length));

    EXPECT_EQ(0, plate::Writer::WriteImpl(
      c.index(), c.offset(), c.length(),
      c.url(), 0, 0));
  }
  // 返回的bundle不正确
  {
    Bundle t;
    t.offset = 0;
    t.index = -231;
    t.length = -1;

    FuseWriter c(t);
    EXPECT_FALSE(c.SetPrefix("fmn04/20010625/1035", 'L'));
  }
}


TEST(Reader,extract) {
  const char* url = "fmn04/20010625/1035/D1/L_A_V_B36e7X.jpg";
  std::string filename, prefix;
  uint32 offset = 0, length = 0, hash = 0;

  EXPECT_TRUE(Reader::Extract(url, &prefix, &filename, NULL, &offset, &length, &hash) == true);
  EXPECT_STREQ("fmn04/20010625/1035", prefix.c_str());
  EXPECT_STREQ("/mnt/mfs/p/31/00000231", filename.c_str());
  EXPECT_STREQ("A", (ToSixty(offset)).c_str());
  EXPECT_STREQ("V", (ToSixty(length)).c_str());

  // 错误情况的测试
  const char* wrongurl1 = "asdfle/203x./cv/_34___34jfajvadsf...gif";
  EXPECT_TRUE(Reader::Extract(wrongurl1, &prefix, &filename, NULL, &offset, &length, &hash) == false);

  const char* wrongurl2 = "fmn04/20010621035/saff322fg/sadf_asdfe_sd32f.jpg";
  EXPECT_TRUE(Reader::Extract(wrongurl2, &prefix, &filename, NULL, &offset, &length, &hash) == false);
}
