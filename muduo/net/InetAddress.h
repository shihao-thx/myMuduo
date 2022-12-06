#ifndef MUDUO_NET_INETADRESS_H_
#define MUDUO_NET_INETADRESS_H_

#include <iostream>
#include <netinet/in.h>
#include <string>

namespace muduo {

namespace net {

class InetAddress {
 public:
  explicit InetAddress(uint16_t port = 0, const std::string& ip = "127.0.0.1");
  explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}
    
  std::string toIP() const;
  std::string toIpPort() const;
  uint16_t toPort() const;

  const sockaddr_in* getSockAddr() const { return &addr_; }
  void setSockAddr(const sockaddr_in& addr) { addr_ = addr; }
 private:
  sockaddr_in addr_;
};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_INETADRESS_H_
