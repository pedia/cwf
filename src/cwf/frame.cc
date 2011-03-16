#include "cwf/frame.h"

#if defined(POSIX) || defined(OS_LINUX)
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <signal.h>
#endif

#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "3rdparty/libfcgi/fcgiapp.h"

#include "base3/common.h"
#include "base3/logging.h"
// #include "base3/logrotate.h"
#include "base3/startuplist.h"
#include "base3/ptime.h"
#include "base3/metrics/stats_counters.h"
#include "base3/logging.h"

#include "cwf/action.h"
#include "cwf/pattern.h"

namespace xce {
extern cwf::User* Authorize(cwf::Request* q);
}

namespace cwf {

static const std::string kDefaultContentType("text/html; charset=utf-8");

void GenerateCommonHeader(Header* header, HttpStatusCode status_code
        , std::string const & message, std::string const& content_type = "") {
  // HTTP/1.x 200 OK
  header->set_status_code(status_code, message);
  if (!content_type.empty())
    header->Add(HH_CONTENT_TYPE, content_type);
}

bool FrameWork::LoadConfig(const std::string& filename) {
  // TODO: 可以实现一个强大的配置体系，支持 so 动态加载
  return true;
}

bool FrameWork::InitHost(bool load_action) {
  // 把 default_action_ 复制到 host_action_
  if (default_actions_)
    host_action_.assign(default_actions_->begin(), default_actions_->end());
  return true;
}

BaseAction* FrameWork::Find(std::string const& url) const {
  for (ActionListType::const_iterator i=host_action_.begin();
    i!= host_action_.end(); ++i) {
      BaseAction* ha = *i;
      if (ha->Match(url))
        return ha;
  }
  return 0;
}

// XAR_IMPL(cwferr);
// XAR_IMPL(cwfall);
// XAR_IMPL(prcGT100);
HttpStatusCode FrameWork::Process(Request* request, Response* response) {
  HttpStatusCode rc;

  base::ptime pt("FrameWork::Process", false, 100);

  BaseAction* a = Find(request->url());
  if (!a) {
    LOG(INFO) << "Not Found: " << request->url();
    ResponseError(HC_NOT_FOUND, "Not Found", response);
    return HC_NOT_FOUND;
  }

  // XAR_INC(cwfall);
  rc = a->Process(request, response);

  if (pt.wall_clock() > 100)
    // XAR_INC(prcGT100);

  if (HC_OK != rc) {
    // XAR_INC(cwferr);
    ResponseError(rc, "Service", response);
    return rc;
  }
  return rc;
}

FrameWork::ActionListType * FrameWork::default_actions_ = 0;

void FrameWork::RegisterAction(BaseAction* a) {
  if (!default_actions_) {
    default_actions_ = new ActionListType();
  }
  default_actions_->push_back(a);
}

void FrameWork::ResponseError(HttpStatusCode code, const char* message, Response* response) {
  // TODO: 貌似返回非 200 有错误, nginx 不识别？
  GenerateCommonHeader(&response->header(), code, message);
  response->OutputHeader();

  // TODO: status code to template file name
#if 0
  ctemplate::TemplateDictionary error_dict_("error"); // TODO: use member?
  ctemplate::Template* tpl = ctemplate::Template::GetTemplate(
    "404.tpl", ctemplate::STRIP_WHITESPACE);
  
  ASSERT(tpl);
  if (tpl)
    tpl->Expand(response, &error_dict_);
#endif
}

void FastcgiProc(FrameWork* fw, int fd) {
  FCGX_Request wrap;
  int ret = FCGX_InitRequest(&wrap, fd, 0);
  ASSERT(0 == ret);

  base::StatsCounter request_count("RequestCount");

  while (FCGX_Accept_r(&wrap) >= 0) {
    // PTIME(pt, "accept", false, 100);

    Request* q = new Request();
    if (!q->Init(wrap.in, wrap.envp)) {
      LOG(ERROR) << "Request::Init failed.";
    }

    Response* p = new Response(wrap.out, wrap.err);

    HttpStatusCode rc = fw->Process(q, p);

    request_count.Increment();
    
#if 0
    std::string r = p->str();
    FCGX_PutStr(r.c_str(), r.size(), wrap.out);
#endif
    // FCGX_Finish_r(&wrap);

    delete p;
    delete q;
  }
}

extern void InstallDefaultAction();

int FastcgiMain(int thread_count, int fd) {
  // xce::Start();

  InstallDefaultAction();

  const char* log_dir = 0;
#if defined(POSIX) || defined(OS_LINUX)
  // find cwf log directory, if you are root user, then /data/cwf/logs, else $HOME/cwf/logs, that's it!
  struct passwd* pw;
  pw = getpwuid(getuid());
  log_dir = (0==strcmp(pw->pw_name, "root")) ? "/data" : pw->pw_dir;
#endif

#if 0
  {
    std::ostringstream ostem;
    if (log_dir) 
      ostem << log_dir << "/cwf/logs/" << getpid() << "/";
    else
      ostem << "/data/cwf/logs/" << getpid() << "/";
		
    // base::LogRotate::instance().Start(ostem.str(), xce::INFO);
  }
#endif

  base::RunStartupList();

#if 0
  {
    LOG(INFO) << "xar::start";
    std::ostringstream ostem;
    if (log_dir) 
      ostem << log_dir << "/cwf/logs/xar.cwf." << getpid();
    else
      ostem << "/data/cwf/logs/xar.cwf." << getpid();
		
    // xce::xar::instance().set_filename(ostem.str());
    // xce::xar::start();
  }
#endif

  // FrameWork::RegisterAction(new EmptyAction());
  // FrameWork::RegisterAction(new TemplateAction());
  // FrameWork::RegisterAction(new xce::FeedTypeAction);

  // bool f = ctemplate::Template::StringToTemplateCache("404.tpl", "{{URL}} Not Found");
  // ASSERT(f);

  std::auto_ptr<FrameWork> fw(new FrameWork());
  if (!fw->LoadConfig("cwf.conf")) {
    LOG(INFO) << "config load failed";
    // ugly link hack
    // xce::FeedTypeAction fta;
    // xce::FriendlyTimeModifier ftm;
    LOG(ERROR) << "TODO: config error\n";
    return 1;
  }

  if (!fw->InitHost(true)) {
    LOG(ERROR) << "TODO: InitHost error reason\n";
    return 1;
  }

  int ret = FCGX_Init();
  ASSERT(0 == ret);
  LOG(INFO) << "FCGX_Init result " << ret;

  boost::thread_group g;
  for (int i=0; i<thread_count; ++i)
    g.add_thread(new boost::thread(
      boost::bind(&FastcgiProc, fw.get(), fd)
    ));

  FastcgiProc(fw.get(), fd); // TODO: 管理 ....
  return 0;
}

}
