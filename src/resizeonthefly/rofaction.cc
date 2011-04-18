#include <string>
#include <fstream>

#include "base3/logging.h"
#include "base3/hashmap.h"
#include "base3/startuplist.h"
#include "base3/globalinit.h"
#include "base3/url.h"
#include "base3/string_split.h"

#include "cwf/action.h"
#include "cwf/frame.h"

#include "resizeonthefly/httpdownload.h"
#include "resizeonthefly/pichelper.h"

namespace rof {

struct HostMap {
  bool Query(const std::string & host, std::string * ip) const {
    MapType::const_iterator i = map_.find(host);
    if (i == map_.end()) 
      return false;

    if (ip)
      *ip = i->second;
    return true;
  }

  bool Reload(const std::string & filename) {
    std::ifstream stem(filename.c_str());
#if 0 // file content
    host=10.3.17.172 fmn.xnimg.cn fmn.rrimg.com fmn.xnpic.com fmn.rrfmn.com
    host=10.3.17.216 fmn.xnimg.cn fmn.rrimg.com fmn.xnpic.com fmn.rrfmn.com
#endif
    std::string line;
    while (std::getline(stem, line)) {
      std::vector<std::string> res;
      base::SplitString(line, ' ', &res);

      std::string ip = res[0];
      {
        std::string::size_type pos = ip.find('=');
        if (pos != std::string::npos)
          ip = ip.substr(pos + 1);
      }
      for (int i=1; i<res.size(); ++i)
        map_.insert(std::make_pair(res[i],ip));
    }
    return true;
  }

  typedef std::hash_map<std::string, std::string> MapType;
  MapType map_;
};

HostMap hostmap_;

static const std::string kImageType("image/jpeg");

struct ResizeAction : public cwf::BaseAction {
  virtual bool Match(const std::string& url) const {
    return url.find("/gi?") == 0;
  }

  virtual cwf::HttpStatusCode Process(cwf::Request * request, cwf::Response * response) {
    // /gi?s=2&op=resize&url=http%3A%2F%2Ffmn.rrfmn.com%2Ffmn047%2F20101130%2F1250%2Fp_main_c37K_32740009d0ab5c40.jpg&w=200&q=80
    // /gi?s=2&op=resize&q=50&w=80&h=120&url=http%3A%2F%2Ffmn.rrfmn.com%2Ffmn052%2F20110417%2F1110%2Fp_head_7sL7_176b000269f45c44.jpg
    // /gi?s=2&op=resize&h=120&w=80&url=http%3A%2F%2Ffmn.xnpic.com%2Ffmn050%2F20110416%2F1210%2Fb_large_oMe0_51c7000887ad5c41.jpg
    // /gi?s=1&op=resize&w=50&h=50&url=http://hdn.xnimg.cn/photos/hdn521/20110331/0045/h_tiny_5Kij_70c3000234a62f75.jpg

    // nginx 10.3.19.212
    // memcached 3
    // gi/gn 
    //     server 10.3.18.202:9092;
    //     server 10.3.19.192:9092;
    //     server 10.3.19.193:9092;

    // s=2
    // op
    // url
    // w,h
    // q

    std::string op = request->query("op");
    if ("crop" != op && "resize" != op) {
      return cwf::HC_BAD_REQUEST;
    }

    std::string photo_url = request->query("url");
    if (photo_url.empty()) {
      return cwf::HC_BAD_REQUEST;
    }

    Url u(photo_url.c_str());
    if (!u.Validate())
      return cwf::HC_BAD_REQUEST;

    int w = request->query("w", 0);
    int h = request->query("h", 0);
    int q = request->query("q", 50);

    if (0 == w)
      return cwf::HC_BAD_REQUEST;

    q = std::max(10, q);
    q = std::min(90, q);

    if ("crop" == op) {
      if (0 == h)
        return cwf::HC_BAD_REQUEST;
    }

    // url
    std::string inner_url("http://");
    {
      // host => ip
      std::string ip;
      if (hostmap_.Query(u.host(), &ip)) {
        inner_url += ip;
        inner_url += u.path();
        if (!u.parameter().empty())
          inner_url += u.parameter();
      }
      else {
        LOG(WARNING) << "inner host failed for " << u.host();
        inner_url = photo_url;
      }
    }

    // retrieve image data
    std::vector<char> buf_for_image;
    buf_for_image.reserve(10 * 1024);
    bool f = HttpDownload(inner_url, &buf_for_image);
    if (!f)
      return cwf::HC_SERVICE_UNAVAILABLE;

    {
      // resize or crop
      AutoImage img;
      if (!img.Init(&buf_for_image[0], buf_for_image.size()))
        return cwf::HC_SERVICE_UNAVAILABLE;

      if ("resize" == op)
        img.Resize(w, q);
      else if("crop" == op)
        img.Crop(w, h);

      // response
      char * buf = 0;
      int length = 0;
      if (!img.WriteToBlob(&buf, &length)) {
        return cwf::HC_INSUFFICIENT_STORAGE;
      }

      response->header().set_status_code(cwf::HC_OK, "OK");
      response->header().Add(cwf::HH_CONTENT_TYPE, kImageType);
#ifdef WIN32
# define snprintf _snprintf
#endif
      char sz[16];
      snprintf(sz, 16, "%d", length);
      response->header().Add(cwf::HH_CONTENT_LENGTH, sz);

      response->WriteRawWithHeader(buf, length);
    }
    
    return cwf::HC_OK;
  }
};

static void Init() {
  cwf::FrameWork::RegisterAction(new ResizeAction);

  hostmap_.Reload("../conf/image_hosts.conf");

  InitMagickLib();
}

}

GLOBAL_INIT(ROFACTION, {
  base::AddStartup(&rof::Init);
});
