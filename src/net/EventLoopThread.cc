#include "src/net/EventLoopThread.h"
#include "src/net/EventLoop.h"

using namespace muduo::net;

EventLoopThread::EventLoopThread(const callback_t<EventLoop*> threadInit, 
                                const std::string& name) : loop_(nullptr),
  exiting_(false), thread_(std::bind(&EventLoopThread::threadFunc, this), name),
  /* mutex_() */ cond_(), threadInitCallback_(threadInit) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  // start a thread first, will create a Loop in it
  thread_.start();

  EventLoop* loop = nullptr;
  {
    // std::lock_guard<std::mutex> lock(mutex_);
    std::unique_lock<std::mutex> lock(mutex_); // dead lock? nope.
    // wait Loop is already created
    // why need while to wait? --avoid spurious wakeup
    while (!loop_) {
      // wait will release the lock
      cond_.wait(lock); 
    }
    loop = loop_;
  }
  return loop;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;
  if (threadInitCallback_) {
    threadInitCallback_(&loop);
  }

  {
    // nessary to add lock here? because already have cond_
    // it's nessary, because the below lock ensure above conn_.wait can be notified
    // good desgin!
    std::lock_guard<std::mutex> lock(mutex_);
    loop_  = &loop;
    cond_.notify_one(); // notify startLoop func can exit
  }

  loop.loop(); // dead while
  std::lock_guard<std::mutex> lock(mutex_);
  loop_ = nullptr;
}