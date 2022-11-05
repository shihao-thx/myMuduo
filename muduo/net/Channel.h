#ifndef MUDUO_NET_CHANNEL_H_
#define MUDUO_NET_CHANNEL_H_ 

#include "muduo/util/noncopyable.h"
#include "muduo/util/callback.h"

namespace muduo {

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

class Channel : noncopyable {
 public:
  Channel(EventLoop* loop, int fd);

  int fd() const { return fd_; }
  int events() const { return events_; }
  void setRevents(int revt) { revents_ = revt; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }
  // will be called by EventLoop::loop() and do something according to revnets_
  void handleEvent(); 

  void setReadCallback(const callback_t<>& rd) { readCallback_ = rd; }
  void setWriteCallback(const callback_t<>& wr) { writeCallback_ = wr; }
  void setErrorCallback(const callback_t<>& err) { errorCallback_ = err; }

  void enableReading() { events_ |= kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }
  void disableReading() { events_ &= ~kReadEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }

  // for Poller
  int index() { return index_; }
  void setIndex(int index) { index_ = index; }

  EventLoop* ownerLoop() { return loop_; }

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

  callback_t<> readCallback_;
  callback_t<> writeCallback_;
  callback_t<> errorCallback_;

};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_CHANNEL_H_
