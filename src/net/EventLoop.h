#ifndef MUDUO_NET_EVENTLOOP_H_
#define MUDUO_NET_EVENTLOOP_H_

#include <atomic>
#include <unistd.h>
#include <memory>
#include <vector>
#include <mutex>

#include "src/util/noncopyable.h"
#include "src/util/Timestamp.h"
#include "src/util/callback.h"
#include "src/util/CurrentThread.h"

namespace muduo {

namespace net {

/*-------------------------------EventLoop Design GUID-----------------------------*\
 * As the principle: one loop per thread, we should promise that only one EventLoop 
 * object in a thread. So the global variable that only belongs to a thread will be 
 * defined by the __thread type to identify this. But why we need the one loop per 
 * thread? Per thread is easy to understand, but one loop? We can consider the loop 
 * as an event loop, which means we will use IO multiplexing in the thread. In 
 * traditional reactor model, there is a accept() event loop in main thread and the 
 * read()/write() event loop in the other thread. The point is how we dispatch the 
 * connfd returned from accept() to the other loop.
 * 
 * In my view, we need a packaging called IO Channel to have all the infomation of 
 * a event including fd, event type, and so on. And read()/write() EventLoop should 
 * have a Channel poll(Map structure) to hold all the registered Channel and we 
 * easily get the Channel through the fd, and then what to do? 
 * B.T.W. All EventLoops and Threads are created by class EventLoopThread.
 */

class Channel;
class Poller;

class EventLoop  : public noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;

  EventLoop();
  ~EventLoop();

  void loop();

  void quit();

  muduo::Timestamp PollReturnTime() const { return pollReturnTime_; } 

  int64_t iteration() const { return iteration_; }

  void runInLoop(callback_t<> callback);

  void queueInLoop(callback_t<> callback);

  size_t queueSize() const;

  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);

  bool isInLoopThread() const { return threadID_ == CurrentThread::tid(); }
  bool eventHandling() const { return eventHandling_; }

  static EventLoop* getEventLoopOfCurrentThread(); 

 private: 
  void handleRead(); // wake up
  void doPendingFunctors();

  bool looping_;
  const pid_t threadID_;
  std::atomic<bool> quit_;
  bool eventHandling_;
  bool callingPendingFunctors_;
  int64_t iteration_;
  muduo::Timestamp pollReturnTime_;
  std::unique_ptr<Poller> poller_;
  // timequeue
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChanel_;

  ChannelList activeChanels_;
  Channel* currentActiveChannel_;

  mutable std::mutex mutex_;
  std::vector<callback_t<>> pendingFunctors_;
};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_EVENTLOOP_H_