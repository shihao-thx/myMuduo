#include "src/net/Socket.h"
#include "glog/logging.h"

#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <fcntl.h>

using namespace muduo::net;

Socket::~Socket() {
  if (::close(sockfd_) < 0) {
    LOG(ERROR) << "Socket::close";
  }
}

bool Socket::getTcpInfo(tcp_info* tcpi) const {
  socklen_t len = sizeof(tcp_info);
  bzero(tcpi, len);
  return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getTcpInfoString(char* buf, int len) const {
  tcp_info tcpi;
  bool ok = getTcpInfo(&tcpi);
  if (ok) {
    snprintf(buf, len, "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
             tcpi.tcpi_rto,          // Retransmit timeout in usec
             tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
             tcpi.tcpi_snd_mss,
             tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost,         // Lost packets
             tcpi.tcpi_retrans,      // Retransmitted packets out
             tcpi.tcpi_rtt,          // Smoothed round trip time in usec
             tcpi.tcpi_rttvar,       // Medium deviation
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
  }
  return ok;
}

void Socket::bindAddress(const InetAddress& addr) {
  if (::bind(sockfd_, (sockaddr*) addr.getSockAddr(), sizeof addr) < 0) {
    LOG(FATAL) << "Socket::bindAddress";
  }
}

void Socket::listen() {
  if (::listen(sockfd_, 1024) < 0) {
    LOG(FATAL) << "SOCKET::listen";
  }
}

int Socket::accept(InetAddress* clientAddr) {
   sockaddr_in addr;
   // len must be initialized to contain the size
   socklen_t len = sizeof addr;
   bzero(&addr, sizeof addr);
   int connfd;
   std::cout << "sockfd_ = " << sockfd_ << std::endl;
   //if ((connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len,
      //SOCK_NONBLOCK | SOCK_CLOEXEC) < 0)) {
  if ((connfd = ::accept(sockfd_, (sockaddr*) &addr, &len)) < 0) {
    LOG(FATAL) << "Socket::accept";
   } else {
    // nonblock
    int flags = ::fcntl(connfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(connfd, F_SETFL, flags);
    // close on exec
    flags = ::fcntl(connfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(connfd, F_SETFD, flags);
    clientAddr->setSockAddr(addr);
   }
   return connfd;
}

void Socket::shutdownWrite() {
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG(ERROR) << "Socket::shutdownWrite";
  }
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on? 1 : 0; 
  if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, 
                  (const void*)&optval, sizeof(int)) < 0) {
    LOG(ERROR) << "Socket::setTcpNoDelay";
  }
}

void Socket::setReuseAddr(bool on) {
  int optval = on? 1 : 0; 
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, 
                  (const void*)&optval, sizeof(int)) < 0) {
    LOG(ERROR) << "Socket::setReuseAddr";
  }
}

void Socket::setReusePort(bool on) {
  int optval = on? 1 : 0; 
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, 
                  (const void*)&optval, sizeof(int)) < 0) {
    LOG(ERROR) << "Socket::setReuesePort";
  }
}

void Socket::setKeepAlive(bool on) {
  int optval = on? 1 : 0; 
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, 
                  (const void*)&optval, sizeof(int)) < 0) {
    LOG(ERROR) << "Socket::setKeepAlive";
  }
}
