#include <iostream>
#include <fstream>

#include "base3/common.h"
#include "base3/getopt_.h"
#include "base3/logging.h"

#include "cwf/frame.h"
#include "cwf/connect.h"

#if defined(POSIX) || defined(OS_LINUX)
#include <unistd.h>
#include <errno.h>
#endif

static void show_help () {
  puts("Usage: cwf [options] [-- <fcgiapp> [fcgi app arguments]]\n" \
    "\n" \
    "Options:\n" \
    " -d <directory> chdir to directory before spawning\n" \
    " -a <address>   bind to IPv4/IPv6 address (defaults to 0.0.0.0)\n" \
    " -p <port>      bind to TCP-port\n" \
    " -t <thread>    thread count\n" \
    " -s <path>      bind to Unix domain socket\n" \
    " -M <mode>      change Unix domain socket mode\n" \
    " -C <children>  (PHP only) numbers of childs to spawn (default: not setting\n" \
    "                the PHP_FCGI_CHILDREN environment variable - PHP defaults to 0)\n" \
    " -F <children>  number of children to fork (default 1)\n" \
    " -P <path>      name of PID-file for spawned process (ignored in no-fork mode)\n" \
    " -n             no fork (for daemontools)\n" \
    " -v             show version\n" \
    " -?, -h         show this help\n" \
    "(root only)\n" \
    " -c <directory> chroot to directory\n" \
    " -S             create socket before chroot() (default is to create the socket\n" \
    "                in the chroot)\n" \
  );
}

int main(int argc, char* argv[]) {
  int port = 3000;
  const char * addr = "0.0.0.0";
  const char * unixsocket = 0;
  int sockmode = 0; // 缺省值从 -1 改为 0 了
  char * fcgi_dir = 0;
  int thread_count = 0;

  int fork_count = 0;

  int o;

  while (-1 != (o = getopt(argc, argv, "a:d:F:M:p:t:s:?h"))) {
    switch(o) {
    case 'd': fcgi_dir = optarg; break;
    case 'a': addr = optarg;/* ip addr */ break;
    case 'p': 
      {
        char *endptr = 0;
        port = strtol(optarg, &endptr, 10);/* port */
        if (*endptr) {
          fprintf(stderr, "spawn-fcgi: invalid port: %u\n", (unsigned int) port);
          return -1;
        }
      }
      break;
    case 't': thread_count = strtol(optarg, NULL, 10);/*  */ break;
    case 'F': fork_count = strtol(optarg, NULL, 10);/*  */ break;
    case 's': unixsocket = optarg; /* unix-domain socket */ break;
//    case 'c': if (i_am_root) { changeroot = optarg; }/* chroot() */ break;
//    case 'u': if (i_am_root) { username = optarg; } /* set user */ break;
//    case 'g': if (i_am_root) { groupname = optarg; } /* set group */ break;
//    case 'U': if (i_am_root) { sockusername = optarg; } /* set socket user */ break;
//    case 'G': if (i_am_root) { sockgroupname = optarg; } /* set socket group */ break;
//    case 'S': if (i_am_root) { sockbeforechroot = 1; } /* open socket before chroot() */ break;
    case 'M': sockmode = strtol(optarg, NULL, 0); /* set socket mode */ break;
//    case 'n': nofork = 1; break;
//    case 'P': pid_file = optarg; /* PID file */ break;
//    case 'v': show_version(); return 0;
    case '?':
    case 'h': show_help(); return 0;
    default:
      show_help();
      return -1;
    }
  }

#if defined(POSIX) || defined(OS_LINUX)
  if (fcgi_dir && -1 == chdir(fcgi_dir)) {
    fprintf(stderr, "spawn-fcgi: chdir('%s') failed: %s\n", fcgi_dir, strerror(errno));
    return -1;
  }
#endif

  int rc = cwf::FastcgiConnect(addr, port, unixsocket, sockmode, fork_count);
#if defined(POSIX) || defined(OS_LINUX)
  // rc = 0 表示parent 执行成功
  // rc = -1 表示 child 成功
  // 其他，失败
  if (rc > 0 && fork_count > 0) {
    return 0;
  }

  // child
  if (rc == -1 && fork_count > 0) {
    return cwf::FastcgiMain(thread_count, 0);
  }
#endif

  return cwf::FastcgiMain(thread_count, rc);
}
