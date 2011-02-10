#ifndef CWF_FRAME_H__
#define CWF_FRAME_H__

#include <string>
#include <list>

#include "cwf/stream.h"

namespace cwf {

struct BaseAction;

class FrameWork {
public:
  bool LoadConfig(const std::string& filename);
  bool InitHost(bool load_action);

  void AddAction(BaseAction* a) {
    host_action_.push_back(a);
  }

  HttpStatusCode Process(Request* request, Response* response);

  static void RegisterAction(BaseAction*);

private:
  BaseAction* Find(std::string const& url) const;

  void ResponseError(HttpStatusCode, const char*, Response*);

  typedef std::list<BaseAction*> ActionListType;

  static ActionListType * default_actions_;

  ActionListType host_action_;
};

int FastcgiMain(int thread_count, int fd);

}
#endif // CWF_FRAME_H__
