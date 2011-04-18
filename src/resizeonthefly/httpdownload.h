#ifndef RESIZEONTHEFLY_HTTPD_H__
#define RESIZEONTHEFLY_HTTPD_H__

#include <string>
#include <vector>

namespace rof {

bool HttpDownload(const std::string& url, std::vector<char>* buf);

}
#endif // RESIZEONTHEFLY_HTTPD_H__
