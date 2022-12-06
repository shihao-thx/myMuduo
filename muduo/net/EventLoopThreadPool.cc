#include "muduo/net/EventLoopThreadPool.h"
#include "muduo/net/EventLoopThread.h"
#include "glog/logging.h"

using namespace muduo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* mainLoop, 
const std::string& nameArg) : mainLoop_(mainLoop), name_(nameArg), started_(false),
  numThreads_(0), next_(0) {
  // google::InitGoogleLogging("myMuduo");
}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const callback_t<EventLoop*> threadInit) {
  started_ = true;

  for (int i = 0; i < numThreads_; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    EventLoopThread* t = new EventLoopThread(threadInit, buf);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());
    
    if (numThreads_ == 0 && threadInit) {
      threadInit(mainLoop_);
    }
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  EventLoop* loop = mainLoop_;
  // round robin algorithm
  if (!loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}
