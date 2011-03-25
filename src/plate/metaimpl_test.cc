#include <gtest/gtest.h>
#include "plate/metaimpl.h"

using namespace plate;

TEST(Interface, Align) {
  EXPECT_EQ(0, Align1K(0));
  EXPECT_EQ(1024, Align1K(1));
  EXPECT_EQ(1024, Align1K(1023));
  EXPECT_EQ(1024, Align1K(1024));
  EXPECT_EQ(2*1024, Align1K(1025));
  EXPECT_EQ(102400, Align1K(102400));
  EXPECT_EQ(102400 + 1024, Align1K(102401));
}

TEST(Interface, Bundle) {
  char name[200];
  BundleFilePath(name, 200, 100);
  std::cout << name;
  EXPECT_TRUE(strlen(name) > 10);
}

TEST(MetaWriter, Write) {
  struct stat st;
  const char* test_file = "test.jpg";
  stat(test_file, &st);

  char * content = new char[st.st_size];
  FILE *fp = fopen(test_file, "rb");
  ASSERT_TRUE(fp != NULL);
  if (fp) {
    fread(content, st.st_size, 1, fp);
    fclose(fp);
  }

  const int COUNT = 2;

  std::string urls[COUNT];

  // 写2次
  BundleAllocator* a = DefaultAllocator();
  int i = 0;
  for (int i=0; i<COUNT; ++i) {
    Writer* aw = a->Allocate(st.st_size);

    char prefix[1024];
    sprintf(prefix, "fnm04/20011034/%04d", i);

    aw->SetPrefix(prefix);
    urls[i] = aw->url();
    std::cout << i << " " << aw->url() << "\n";

    int ret = Writer::WriteImpl(
      aw->index(), aw->offset(), aw->length(),
      aw->url(), content, st.st_size);
    // std::cout << ret << "\n";
    EXPECT_EQ(st.st_size, ret);

    delete aw;
  }

  // 验证读
  EXPECT_EQ(st.st_size, Reader::Read(urls[0].c_str(), 0, 0));

  char * read_buffer = new char[st.st_size];
  EXPECT_EQ(st.st_size, Reader::Read(urls[0].c_str(), read_buffer, st.st_size));
  EXPECT_EQ(0, memcmp(content, read_buffer, st.st_size));

  for (int i=1; i<COUNT; ++i) {
    EXPECT_EQ(st.st_size, Reader::Read(urls[i].c_str(), read_buffer, st.st_size));
    EXPECT_EQ(0, memcmp(content, read_buffer, st.st_size));
  }

  // Write with userdata
  {
    Writer* aw = a->Allocate(st.st_size);

    aw->SetPrefix("fnm04/20011034/hello");
    std::string url = aw->url();
    std::cout << "with userdata: " << aw->url() << "\n";
    Writer::WriteImpl(
      aw->index(), aw->offset(), aw->length(),
      aw->url(), content, st.st_size, "hello\0", 6);

    delete aw;

    // read
    std::string prefix, filename;
    uint32 offset, length, hash;
    char type;
    EXPECT_TRUE(Reader::Extract(url.c_str(), &prefix, &filename
      , &type
      , &offset, &length, &hash));

    char filepath[1024];
    BundleFilePath(filepath, 1024, atol(filename.c_str()));
    EXPECT_EQ(st.st_size, length);
  }

  delete [] content;
  delete [] read_buffer;
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc,argv); 
  int r = RUN_ALL_TESTS();
  return r;
}
