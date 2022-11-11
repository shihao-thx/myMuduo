#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H_
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H_

#include "muduo/util/noncopyable.h"
#include "muduo/util/callback.h"

#include <string>
#include <memory>
#include <vector>

namespace muduo {

namespace net {

/*-------------------------------Thread Design GUID-----------------------------*\
 * class EventLoopThread is used to manage EventLoop.
 */

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_  = numThreads; }
  void start(const callback_t<EventLoop*> threadInit/* = callback_t<>() */); // TODO

  // round-robin to distribute channel to Loop 
  EventLoop* getNextLoop();

  bool started() const { return started_; } 

  const std::string& name() const { return name_; } 

 private:
  EventLoop* baseLoop_;
  std::string name_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

} // namespace net

} // nemaspace muduo

#endif // !MUDUO_NET_EVENTLOOPTHREADPOOL_H_