#include "resizeonthefly/httpdownload.h"

#ifdef OS_WIN
#include <fstream>

namespace rof {
bool HttpDownload(const std::string& url, std::vector<char>* buf) {
  std::ifstream stem("test.jpg", std::ios::binary);
  stem.unsetf(std::ios::skipws);
  std::copy(
    std::istream_iterator<char>(stem), std::istream_iterator<char>()
    , std::back_inserter(*buf)
    );
  return true;
}
}
#else
#include "urdl/read_stream.hpp"

namespace rof {

// ugly global
boost::asio::io_service io_service;

bool HttpDownload(const std::string& url, std::vector<char>* buf) {
  try {
    urdl::read_stream stream(io_service);
    stream.open(url.c_str());

    char data[1024];
    while (true) {
      boost::system::error_code ec;
      std::size_t length = stream.read_some(boost::asio::buffer(data), ec);
      if (ec == boost::asio::error::eof)
        break;

      if (ec)
        return false;

      if (buf)
        buf->insert(buf->end(), data, data + length);
    } 
    return true;
  } catch (std::exception & e) {
    return false;
  } 
}

}
#endif
