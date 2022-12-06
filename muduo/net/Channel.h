#ifndef MUDUO_NET_CHANNEL_H_
#define MUDUO_NET_CHANNEL_H_ 

#include "muduo/util/noncopyable.h"
#include "muduo/util/callback.h"

#include <memory>

namespace muduo {

class Timestamp;

namespace net {

/*-------------------------------Channel Design GUID-----------------------------*\
 * Why should we have the class Channel? In the other words, what's the purpose of
 * designing the class? According to one (events)loop per thread, a event only be-
 * long to a loop and also a thread. So we can pack the information of the event as 
 * the Channel used for transferring to class Eventloop and Poller. So, class 
 * Channel is just about anything of a event and doesn't have any race condition in 
 * it. The connfd and event type, etc. we care should be initialized when it was 
 * constructed and we can do something by event type happening. So the class 
 * Channel gives the callback method and the callback behaviors of the Channel only 
 * depends on other classes using it.
 */

class EventLoop;
// class Timestamp;

class Channel : noncopyable {
 public:
  Channel(EventLoop* loop, int fd);
  ~Channel();

  int fd() const { return fd_; }
  int events() const { return events_; }
  std::string eventsToString();
  void setRevents(int revents) { revents_ = revents; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }
  // will be called by EventLoop::loop() and do something according to revnets_
  // currentActiveChannel_->handleEvent()
  void handleEvent(Timestamp receiveTime); 

  void setReadCallback(const callback_t<Timestamp>& rd) { readCallback_ = rd; }
  void setWriteCallback(const callback_t<>& wr) { writeCallback_ = wr; }
  void setCloseCallback(const callback_t<>& cl) { closeCallback_ = cl; }
  void setErrorCallback(const callback_t<>& err) { errorCallback_ = err; }
  // std::weak_ptr<void> tie_ to probe TcpConnection is still alive
  void tie(const std::shared_ptr<void>&);

  void enableReading() { events_ |= kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }
  void disableReading() { events_ &= ~kReadEvent; update(); }
  void disableWriting() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isReading() const { return events_ & kReadEvent; }

  // for Poller
  int index() { return index_; }
  const char* indexToString();
  void setIndex(int index) { index_ = index; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  // update() -> EventLoop::updateChannel() -> Poller::updateChannel()
  void update(); 

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;  // event type we care
  int revents_; // active event type
  int index_;   // used by Poller

  std::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  bool addedToLoop_;

  callback_t<Timestamp> readCallback_;
  callback_t<> writeCallback_;
  callback_t<> closeCallback_;
  callback_t<> errorCallback_;
};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_CHANNEL_H_
