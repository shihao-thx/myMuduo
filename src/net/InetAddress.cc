#include "src/net/InetAddress.h"

#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

using namespace muduo::net;

InetAddress::InetAddress(uint16_t port, const std::string& ip) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  // transfer port and ip to Inet bytes with htons and inet_addr
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = inet_addr(ip.c_str()); 
}
  
std::string InetAddress::toIP() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  return buf;
}

std::string InetAddress::toIpPort() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  uint16_t port = ntohs(addr_.sin_port);
  auto len = strlen(buf);
  sprintf(buf + len, ":%u", port);
  return buf;
}

uint16_t InetAddress::toPort() const {
  return ntohs(addr_.sin_port);
}
