#include "muduo/net/TcpServer.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/Acceptor.h"
#include "muduo/net/EventLoopThreadPool.h"
#include "EventLoop.h"
#include "glog/logging.h"

#include <iostream>
#include <string.h>

using namespace muduo::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
  const std::string& serverName, Option option) : 
  loop_(check_loop_not_null(loop)), ipPort_(listenAddr.toIpPort()), name_(serverName),
  acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)), 
  threadPool_(new EventLoopThreadPool(loop, name_)),
  connectionCallback_(std::bind(defaultConnectionCallback, std::placeholders::_1)),
  messageCallback_(std::bind(defaultMessageCallback, std::placeholders::_1, 
  std::placeholders::_2, std::placeholders::_3)), nextConnID_(1), started_(false) {
  acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
    std::placeholders::_1, std::placeholders::_2)); 
}

TcpServer::~TcpServer() {
  DLOG(INFO) << "TcpServer::~TcpServer [" << name_ << "] destructing";
  for (auto& item : connections_) {
    TcpConnectionPtr conn(item.second);
    item.second.reset(); // release TcpConnection as soon as possible?
    conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    //item.second->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, item.second));
    //item.second.reset();
  }
}

void TcpServer::setThreadNum(int numThreads) {
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
  if (!started_.load()) {
    threadPool_->start(threadInitCallback_);
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

void TcpServer::newConnection(int connfd, const InetAddress& clientAddr) {
  EventLoop* ioLoop = threadPool_->getNextLoop();
  char buf[64] = {0};
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnID_);
  ++nextConnID_;
  std::string connName = name_ + buf;
  LOG(INFO) << "TcpServer::newConnection [" << name_ << "] - new connection ["
            << connName << "] from " << clientAddr.toIpPort(); 
  // get serverAddr
  sockaddr_in server;
  ::bzero(&server, sizeof server);
  socklen_t addrLen = sizeof server;
  if (::getsockname(connfd, (sockaddr*)&server, &addrLen) < 0) {
    LOG(ERROR) << "TcpServer::newConnection::getsockname";
  }
  InetAddress serverAddr(server);
  // using make_shared will delay memory free, due to weak_ptr counting
  auto conn = std::make_shared<TcpConnection>(ioLoop, connName,
    connfd, serverAddr, clientAddr);
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  LOG(INFO) << "TcpServer::removeConnectionInLoop [" << name_
            << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}