#ifndef MUDUO_NET_TCPCONNECTION_H_
#define MUDUO_NET_TCPCONNECTION_H_

#include "src/util/noncopyable.h"
#include "src/net/InetAddress.h"
#include "src/util/callback.h"
#include "src/util/Timestamp.h"
#include "src/net/Buffer.h"

#include <atomic>
#include <netinet/tcp.h>

namespace muduo {

namespace net {

class EventLoop;
class InetAddress;
class Socket;
class Channel;
/*--------------------------TcpConnection Design GUID-----------------------------*\
 * One loop per thread means a eventloop for a TcpConnection can only run in the 
 * same thread, and a thread can have other loops(connections). So how to dis-
 * tinguish I/O events in a thread? We can pack all sources of a event as TcpConnection like 
 * confd, buffer, etc. so that 
 */

class TcpConnection : noncopyable, 
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
    const InetAddress& serverAddr, const InetAddress& clientAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& serverAddress() const { return serverAddr_; }
  const InetAddress& clientAddress() const { return clientAddr_; }
  bool connected() { return connected_.load(); }
  
  bool getTcpInfo(tcp_info*) const;
  std::string getTcpInfoString() const;

  void send(const std::string& message);
  //void send(std::string&& message);
  //void send(Buffer&& mssage);
  void shutdown();

  void startRead();
  void stopRead();
  bool isReading() const { return reading_; }

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

  void setHighWaterMarkCallback(const callback_t<const TcpConnectionPtr&, size_t>& callback,
      size_t highWaterMark) {
    highWaterMarkCallback_ = callback;
    highWaterMark_ = highWaterMark;
  }

  void setCloseCallback(const callback_t<const TcpConnectionPtr&>& callback) {
    closeCallback_ = callback;
  }

  void connectEstablished();
  void connectDestroyed();

 private:
  enum State { kConnecting, kDisconnecting };
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const std::string& message);
  void shutdownInLoop();
  void forceCloseInLoop();
  void setState(State s) { state_ = s; }
  const char* stateTostring();
  void startReadInLoop();
  void stopReadInLoop();

  EventLoop* loop_;
  const std::string name_;
  const InetAddress serverAddr_;
  const InetAddress clientAddr_;
  State state_;
  std::atomic<bool> connected_;
  bool reading_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;

  callback_t<const TcpConnectionPtr&> connectionCallback_;
  callback_t<const TcpConnectionPtr&, Buffer*, Timestamp> messageCallback_;
  callback_t<const TcpConnectionPtr&> writeCompleteCallback_;
  callback_t<const TcpConnectionPtr&, size_t> highWaterMarkCallback_;
  callback_t<const TcpConnectionPtr&> closeCallback_;
  
  size_t highWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;
};
  
} // namespace net 

} // namespace muduo

#endif // !MUDUO_NET_TCPCONNECTION_H_