#ifndef MUDUO_NET_EVENTLOOPTHREAD_H_
#define MUDUO_NET_EVENTLOOPTHREAD_H_

#include "muduo/util/noncopyable.h"
#include "muduo/util/callback.h"
#include "muduo/util/Thread.h"

#include <mutex>
#include <condition_variable>
#include <string>

namespace muduo {

namespace net {

/*-------------------------------Thread Design GUID-----------------------------*\
 * class EventLoopThread is used to create a loop in a thrad
 */

class EventLoop;

class EventLoopThread : noncopyable {
 public:
  EventLoopThread(const callback_t<EventLoop*> threadInit, 
    const std::string& name = std::string());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();
  
  EventLoop* loop_;
  Thread thread_;

  bool exiting_;
  std::mutex mutex_; 
  std::condition_variable cond_;
  callback_t<EventLoop*> callback_;
};

} // namespace net

} // namespace muduo


#endif // !MUDUO_NET_EVENTLOOPTHREAD_H_