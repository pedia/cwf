#include "base3/startuplist.h"
#include "base3/globalinit.h"

#include "cwf/action.h"
#include "cwf/frame.h"
#include "plate/metaimpl.h"

using namespace cwf;

namespace plate {

struct PlateAction : public BaseAction {
  virtual bool Match(const std::string& url) const {
    return url.find("plate") != std::string::npos;
  }

  virtual HttpStatusCode Process(Request * request, Response * response) {
    static const std::string kDefaultContentType("text/html; charset=utf-8");
    response->header().set_status_code(HC_OK, "OK");
    response->header().Add(HH_CONTENT_TYPE, kDefaultContentType);
    response->OutputHeader();

    BundleAllocator* a = DefaultAllocator();

    const Request::DispositionArrayType & files = request->files();
    for (unsigned int i = 0; i < files.size(); ++i) {
      const cwf::Request::FormDisposition & fd = files[i];

      Writer* aw = a->Allocate("test", fd.data.size());

      aw->SetUrlDetail("test", 0, 'B', "jpg");
      int ret = aw->Write(fd.data.c_str(), fd.data.size());

      if (ret) {
        std::string url("/p/");
        url += aw->url();
        url += "\n";
        response->WriteRaw(url);
      }

      a->Return(aw);
    }
    return HC_OK;
  }
};

static void Init() {
  FrameWork::RegisterAction(new PlateAction);
}

}

GLOBAL_INIT(PLATEACTION, {
  base::AddStartup(&plate::Init);
});
