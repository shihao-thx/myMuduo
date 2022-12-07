#ifndef MUDUO_NET_ACCEPTOR_H_
#define MUDUO_NET_ACCEPTOR_H_

#include "src/util/noncopyable.h"
#include "src/net/Socket.h"
#include "src/net/Channel.h"
#include "src/util/callback.h"

namespace muduo {

namespace net {

class EventLoop;
class InetAddress;

class Acceptor : noncopyable {
 public:
  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const callback_t<int, const InetAddress&> callback) {
    newConnectionCallback_ = callback;
  }

  void listen();

  bool listening() const { return listening_; }

 private:
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  callback_t<int, const InetAddress&>  newConnectionCallback_;
  bool listening_;
  int fd_;
};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_ACCEPTOR_H_
