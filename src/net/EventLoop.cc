#include "src/net/EventLoop.h"
#include "src/util/CurrentThread.h"
#include "src/net/poller/EpollPoller.h"
#include "src/net/Channel.h"
#include "glog/logging.h"

#include <assert.h>
#include <sys/eventfd.h>

using namespace muduo;
using namespace muduo::net;

__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

static int createEventFd() {
  // eventfd() returns a new file descriptor that can be used to refer to 
  // the eventfd object. 
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG(FATAL) << "Failed in eventfd";
  }
  return evtfd;
}

EventLoop::EventLoop() : looping_(false), threadID_(CurrentThread::tid()),
  quit_(false), eventHandling_(false), callingPendingFunctors_(false), 
  iteration_(0), poller_(Poller::newDefaultPoller(this)), 
  wakeupFd_(createEventFd()), wakeupChanel_(new Channel(this, wakeupFd_)), 
  currentActiveChannel_(nullptr) {
  DLOG(INFO) << "EventLoop created " << this << " in thread " << threadID_; 
  if (t_loopInThisThread) {
    LOG(FATAL) << "Another EventLoop " << t_loopInThisThread 
      << " exists in this thread " << threadID_;
  } else {
    t_loopInThisThread = this;
  }

  wakeupChanel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChanel_->enableReading(); // always read the wakeupFd
}

EventLoop::~EventLoop() {
  DLOG(INFO) << "EventLoop " << this << " of thread " << threadID_ 
    << "destructs in thread " << CurrentThread::tid();
  wakeupChanel_->disableAll();
  wakeupChanel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  looping_ = true;
  quit_ = false;
  DLOG(INFO) << "EventLoop " << this << " start looping";
  while (!quit_) {
    activeChanels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChanels_);
    ++iteration_;
    eventHandling_ = true;

    for (auto channel : activeChanels_) {
      currentActiveChannel_ = channel;
      // to run callback
      currentActiveChannel_->handleEvent(pollReturnTime_); // call sync func
    }

    currentActiveChannel_ = nullptr;
    eventHandling_ = false;
    // 
    doPendingFunctors();
  }
  DLOG(INFO) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

// to run callbacks in waiting queue
void EventLoop::doPendingFunctors() {
  std::vector<callback_t<>> functors;
  callingPendingFunctors_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    // good design: all elements in pendingFunctors_ thansfer to tempt functors
    // so that memory pengdingFunctors_ occupied can be released
    functors.swap(pendingFunctors_);
  }

  for (const callback_t<> func : functors) {
    func();
  }
  callingPendingFunctors_ = false; 

}

void EventLoop::queueInLoop(callback_t<> callback) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(std::move(callback));
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();  
  }
} 

size_t EventLoop::queueSize() const {
  // mutex_ needs to be mutable
  std::lock_guard<std::mutex> lock(mutex_);
  return pendingFunctors_.size();
}

void EventLoop::runInLoop(callback_t<> callback) {
  if (isInLoopThread()) {
    callback();
  } else {
    // Thread ID of the eventLoop != thread ID of running callback()
    // ensure callback to be called in subloop rather than mainloop.
    // Usually use queueInLoop when accepting a new connection.
    queueInLoop(std::move(callback));
  }
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

// the EventLoop thread maybe sleeping due to epoll_wait
void EventLoop::wakeup() {
  uint64_t one = 1; // one byte
  ssize_t n = ::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG(ERROR) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG(ERROR) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::updateChannel(Channel* channel) {
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
  return poller_->hasChannel(channel);
}