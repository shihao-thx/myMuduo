#include "muduo/net/Acceptor.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/EventLoop.h"
#include "glog/logging.h"

using namespace muduo::net;

static int createNonBlocking() {
  int sockfd;
  if ((sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)) < 0) {
    LOG(FATAL) << "Acceptor::createNonBlocking";
  }
  return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
  : loop_(loop), acceptSocket_(createNonBlocking()), 
    acceptChannel_(loop, acceptSocket_.fd()), listening_(false) {
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reuseport);
  acceptSocket_.bindAddress(listenAddr);
  // Events dirve: when a new connection is coming, handleRead will be called 
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  acceptChannel_.remove();
}

void Acceptor::listen() {
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  InetAddress clientAddr;
  int connfd = acceptSocket_.accept(&clientAddr);
  if (newConnectionCallback_) {
    newConnectionCallback_(connfd, clientAddr);
  } else {
    ::close(connfd);
  }
}