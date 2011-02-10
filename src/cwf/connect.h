#ifndef CWF_CONNECT_H__
#define CWF_CONNECT_H__

// open socket
// redirect stdin/stdout to socket
    
namespace cwf {

int FastcgiConnect(const char *addr, unsigned short port, const char *unixsocket = 0
                    , /*uid_t uid, gid_t gid, */int mode = 0, int fork_count = 0);

}
#endif // CWF_CONNECT_H__
