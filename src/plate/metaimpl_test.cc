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

struct DummyWriter : public plate::Writer {
  int Write(const char* buf, int size
    , const char *userdata = 0, size_t userdata_size = 0) {
      return 0;
  }

  DummyWriter(int id, size_t offset, size_t length) 
    : Writer(id, offset, length) {}
};

TEST(Interface, Prefix) {
  int align_offset = Align1K(10000);
  DummyWriter dw(321, align_offset, 0xf888);
  EXPECT_TRUE(dw.SetUrlDetail("foo"));
  std::string url = dw.url();
  EXPECT_TRUE(url.find("foo") == 0);
  EXPECT_TRUE(url.find(ToSixty(321)) != std::string::npos); // Sixty(321) = FW
  EXPECT_TRUE(url.find(ToSixty(0xf888)) != std::string::npos); // Sixty(0xf888) = FW
  std::cout << url << std::endl;

  std::string prefix, filename;
  char type;
  uint32 offset, length, hash;
  bool f = Reader::Extract(url.c_str(), &prefix, &filename, 
    &type,
    &offset, &length, &hash);
  EXPECT_TRUE(f);
  EXPECT_EQ(dw.length(), length);
  EXPECT_EQ(align_offset, offset);
  EXPECT_EQ('R', type);
  EXPECT_EQ("foo", prefix);
  // url = foo/20110329/1524/FW/R_C0q_SqZ_DWZOQV.jpg
  std::cout << "prefix: " << prefix << std::endl;
  std::cout << "filename: " << filename << std::endl;
  std::cout << "type: " << type << std::endl;

  EXPECT_FALSE(Reader::Extract("foo/20110329/1523/FW/R_C0q_SqZ_DWZOQV.jpg"));
  EXPECT_FALSE(Reader::Extract("foa/20110329/1524/FW/R_C0q_SqZ_DWZOQV.jpg"));
  EXPECT_FALSE(Reader::Extract("foo/20210329/1524/FW/R_C0q_SqZ_DWZOQV.jpg"));
  EXPECT_FALSE(Reader::Extract("foo/20110329/1534/FW/R_C0q_SqZ_DWZOQV.jpg"));
  EXPECT_FALSE(Reader::Extract("foo/20110329/1534/FW/R_C0q_SqZ_DWZOQE.jpg"));

  EXPECT_FALSE(Reader::Extract("foo/20110329/1524/FW/R_C0q_SqE_DWZOQV.jpg"));
  EXPECT_FALSE(Reader::Extract("foo/20110329/1524/FW/R_C0f_SqZ_DWZOQV.jpg"));
  EXPECT_FALSE(Reader::Extract("foo/20110329/1524/FW/R_C1q_SqZ_DWZOQV.jpg"));
  EXPECT_FALSE(Reader::Extract(""));
  EXPECT_FALSE(Reader::Extract(0));
}

TEST(Interface, Bundle) {
  std::cout << BundleFilename(100, "20100315/1259") << std::endl;
  std::cout << BundleFilename(100) << std::endl;

  for (int i=0; i<5024; ++i) {
    // std::cout << BundleFilename(i) << std::endl;
  }
}

TEST(MetaWriter, Stage) {
  BundleAllocator* a = DefaultAllocator();
  const int COUNT = 405;
  Writer* arr[COUNT];

  // use next stage
  for (int i=0; i<COUNT; i++) {
    arr[i] = a->Allocate("foo", 1000);
    std::cout << arr[i]->index() << " ";
  }
  std::cout << std::endl;
  for (int i=0; i<COUNT; i++)
    a->Return(arr[i]);

  // reuse 1st stage
  for (int i=0; i<COUNT; i++) {
    arr[i] = a->Allocate("foo", 1000);
    std::cout << arr[i]->index() << " ";
    a->Return(arr[i]);
  }
  std::cout << std::endl;
}

TEST(MetaWriter, Allocate) {
  BundleAllocator* a = DefaultAllocator();
  Writer* aw = a->Allocate("foo", 1000);

  int last_id = aw->index();
  EXPECT_GE(aw->offset(), kBundleHeaderSize); // bundle header
  EXPECT_EQ(1000, aw->length());
  int last_length = aw->length();
  int last_offset = aw->offset();
  {
    std::string buf(1000, 'c');
    int ret = aw->Write(buf.c_str(), buf.size());
    EXPECT_EQ(1000, ret);
  }

  a->Return(aw);

  // 
  aw = a->Allocate("foo", 1300);

  int this_id = aw->index();
  EXPECT_EQ(1300, aw->length());
  EXPECT_GE(aw->offset(), kBundleHeaderSize);
  EXPECT_EQ(last_id, this_id);
  last_id = this_id;
  {
    std::string buf(1000, 'c');
    int ret = aw->Write(buf.c_str(), buf.size());
    EXPECT_EQ(1000, ret);
  }

  a->Return(aw);

  // 0
  aw = a->Allocate("foo", 0);
  this_id = aw->index();
  EXPECT_EQ(0, aw->length());
  EXPECT_GE(aw->offset(), kBundleHeaderSize);
  EXPECT_EQ(last_id, this_id);
  {
    int ret = aw->Write("", 0);
    EXPECT_EQ(0, ret);
  }

  a->Return(aw);
}

TEST(MetaWriter, Write) {
  struct stat st;
  const char* test_file = "test.jpg";
  stat(test_file, &st);

  // 1 read image file
  char * content = new char[st.st_size];
  {
    FILE *fp = fopen(test_file, "rb");
    ASSERT_TRUE(fp != NULL);
    if (fp) {
      fread(content, st.st_size, 1, fp);
      fclose(fp);
    }
  }

  BundleAllocator* a = DefaultAllocator();

  // 2 写2次
  const int COUNT = 2;
  std::string urls[COUNT];
  
  int i = 0;
  for (int i=0; i<COUNT; ++i) {
    Writer* aw = a->Allocate("foo", st.st_size);
    EXPECT_TRUE(aw->SetUrlDetail("foo", 0, 'M', "jpg"));
    urls[i] = aw->url();
    std::cout << i << " " << aw->url() << "\n";

    int ret = aw->Write(content, st.st_size);
    EXPECT_EQ(st.st_size, ret);

    delete aw;
  }

  // 验证读
  for (int i=0; i<COUNT; ++i) {
    EXPECT_EQ(st.st_size, Reader::Read(urls[i].c_str(), 0, 0));

    char * read_buffer = new char[st.st_size];
    EXPECT_EQ(st.st_size, Reader::Read(urls[i].c_str(), read_buffer, st.st_size));
    EXPECT_EQ(0, memcmp(content, read_buffer, st.st_size));

    delete [] read_buffer;
  }

  // Write with userdata
  {
    Writer* aw = a->Allocate("fnm04", st.st_size);

    EXPECT_TRUE(aw->SetUrlDetail("fnm04"));
    std::string url = aw->url();
    std::cout << "with userdata: " << aw->url() << "\n";
    int ret = aw->Write(content, st.st_size);

    delete aw;

    // read
    std::string prefix, filename;
    uint32 offset, length, hash;
    char type;
    EXPECT_TRUE(Reader::Extract(url.c_str(), &prefix, &filename
      , &type
      , &offset, &length, &hash));

    // char filepath[1024];
    // BundleFilePath(filepath, 1024, atol(filename.c_str()));
    EXPECT_EQ(st.st_size, length);
  }

  delete [] content;
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc,argv); 
  int r = RUN_ALL_TESTS();
  return r;
}
