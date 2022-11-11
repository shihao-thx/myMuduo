#include "muduo/net/EventLoopThread.h"
#include "muduo/net/EventLoop.h"

using namespace muduo::net;

EventLoopThread::EventLoopThread(const callback_t<EventLoop*> threadInit, 
                                const std::string& name) : loop_(nullptr),
  exiting_(false), thread_(std::bind(&EventLoopThread::threadFunc, this), name),
  /* mutex_() */ cond_(), callback_(threadInit) {}

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
    //std::lock_guard<std::mutex> lock(mutex_);
    std::unique_lock<std::mutex> lock(mutex_);
    // wait Loop is already created 
    while (!loop_) { // why need while to wait
      cond_.wait(lock); 
    }
    loop = loop_;
  }
  return loop;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;
  if (callback_) {
    callback_(&loop);
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_  = &loop;
    cond_.notify_one(); // notify startLoop func can exit
  }

  loop.loop();
  std::lock_guard<std::mutex> lock(mutex_);
  loop_ = nullptr;
}