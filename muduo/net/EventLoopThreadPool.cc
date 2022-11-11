#include "muduo/net/EventLoopThreadPool.h"
#include "muduo/net/EventLoopThread.h"

using namespace muduo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, 
const std::string& nameArg) : baseLoop_(baseLoop), name_(nameArg), started_(false),
  numThreads_(0), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const callback_t<EventLoop*> threadInit) {
  started_ = true;

  for (int i = 0; i < numThreads_; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), 1);
    EventLoopThread* t = new EventLoopThread(threadInit, buf);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());
    
    if (numThreads_ == 0 && threadInit) {
      threadInit(baseLoop_);
    }
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  EventLoop* loop = baseLoop_;

  if (!loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}
