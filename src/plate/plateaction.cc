#include "base3/startuplist.h"
#include "base3/globalinit.h"

#include "cwf/action.h"
#include "cwf/frame.h"

using namespace cwf;

namespace plate {

struct PlateAction : public BaseAction {
  virtual bool Match(const std::string& url) const {
    return false;
  }
  virtual HttpStatusCode Process(Request*, Response*) {
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
