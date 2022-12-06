#ifndef MUDUO_NET_SOCKET_H_
#define MUDUO_NET_SOCKET_H_

#include "muduo/util/noncopyable.h"
#include "muduo/net/InetAddress.h"

#include <netinet/tcp.h>

namespace muduo {

namespace net {
/*-------------------------------Channel Design GUID-----------------------------*\
 * socket interface of Server: 
 *        int socket(int damain, int type, int protocol); return listenfd
 *                                  |
 *                                  V
 *    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, const void*, int len);
 *                                  |
 *                                  V
 *   int bind(int listenfd, const struct sockaddr* addr, socklen_t addrlen);
 *                                  |
 *                                  V
 *                int listen(int listenfd, int backlog)
 */

class Socket : public noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  int fd() const { return sockfd_; };

  bool getTcpInfo(tcp_info* tcpi) const;
  bool getTcpInfoString(char* buf, int len) const;

  void bindAddress(const InetAddress& addr);
  void listen();
  int accept(InetAddress* clientAddr);

  void shutdownWrite();

  // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  void setTcpNoDelay(bool on);

  // Enable/disable SO_REUSEADDR
  void setReuseAddr(bool on);

  // Enable/disable SO_REUSEPORT
  void setReusePort(bool on);

  // Enable/disable SO_KEEPALIVE
  void setKeepAlive(bool on);
 private:
  const int sockfd_;
};

} // namespace net

} // namespace muduo 

#endif // !MUDUO_NET_SOCKET_H_