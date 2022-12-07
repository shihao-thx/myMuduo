#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H_
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H_

#include "src/util/noncopyable.h"
#include "src/util/callback.h"

#include <string>
#include <memory>
#include <vector>

namespace muduo {

namespace net {

/*-------------------------------Thread Design GUID-----------------------------*\
 * class EventLoopThreadPool is used to manage EventLoopThread. It's created by
 * class TcpServer.
 */

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  EventLoopThreadPool(EventLoop* mainLoop, const std::string& nameArg);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_  = numThreads; }
  void start(const callback_t<EventLoop*> threadInit = callback_t<EventLoop*>());

  // round-robin to distribute channel to Loop 
  EventLoop* getNextLoop();

  bool started() const { return started_; } 

  const std::string& name() const { return name_; } 

 private:
  EventLoop* mainLoop_;
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