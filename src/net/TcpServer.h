#ifndef MUDUO_NET_TCPSERVER_H_
#define MUDUO_NET_TCPSERVER_H_

#include "src/net/TcpConnection.h"

#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

namespace muduo {

namespace net {

class EventLoop;
class Acceptor;
class InetAddress;
class EventLoopThreadPool;
class Buffer;
/*-------------------------------TcpServer Design GUID-----------------------------*\
 * TcpServer is the main class users directly use. As a server, it can wait conn-
 * ection with client to establish and take action when read event or write event
 * is coming. But what to do depends on callback users set. 
 */

class TcpServer : noncopyable {
 public:
  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& serverName,
      Option option = kNoReusePort);
  ~TcpServer();

  const std::string& ipPort() const { return ipPort_; }
  const std::string& name() const { return name_; }
  EventLoop* getLopp() const { return loop_; }

  void setThreadNum(int numThreads);

  void setThreadInitCallback(const callback_t<EventLoop*>& callback) {
    threadInitCallback_ = callback;
  }
  std::shared_ptr<EventLoopThreadPool> thradPool() { return threadPool_; }
  
  void start();

  void setConnectionCallback(const callback_t<const TcpConnectionPtr&>& callback) {
    connectionCallback_ = callback;
  }

  void setMessageCallback(
    const callback_t<const TcpConnectionPtr&, Buffer*, Timestamp>& callback) {
    messageCallback_ = callback;
  }

  void setWriteCompleteCallback(const callback_t<const TcpConnectionPtr&>& callback) {
    writeCompleteCallback_ = callback;
  }


 private:
  void newConnection(int sockfd, const InetAddress& clientAddr);
  void removeConnection(const TcpConnectionPtr& conn);
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  EventLoop* loop_; // user defined

  const std::string ipPort_;
  const std::string name_;

  std::unique_ptr<Acceptor> acceptor_;

  std::shared_ptr<EventLoopThreadPool> threadPool_;

  callback_t<const TcpConnectionPtr&> connectionCallback_;
  callback_t<const TcpConnectionPtr&, Buffer*, Timestamp> messageCallback_;
  callback_t<const TcpConnectionPtr&> writeCompleteCallback_;
  callback_t<EventLoop*> threadInitCallback_;

  std::atomic<bool> started_;
 
  int nextConnID_;
  std::unordered_map<std::string, TcpConnectionPtr> connections_;
};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_TCPSERVER_H_

