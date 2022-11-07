#include "muduo/net/EventLoop.h"
#include "muduo/util/CurrentThread.h"
#include "muduo/net/poller/EpollPoller.h"
#include "muduo/net/Channel.h"

#include <assert.h>
#include <sys/eventfd.h>

using namespace muduo;
using namespace muduo::net;

__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventFd() {
  // eventfd() returns a new file descriptor that can be used to refer to 
  // the eventfd object. 
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    // LOG
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop() : looping_(false), threadID_(CurrentThread::tid()),
  quit_(false), eventHandling_(false), callingPendingFunctors_(false), 
  iteration_(0), poller_(Poller::newDefaultPoller(this)), 
  wakeupFd_(createEventFd()), wakeupChanenl_(new Channel(this, wakeupFd_)), 
  currentActiveChannel_(nullptr) {
  // Log
  if (t_loopInThisThread) {
    // FATAL
  } else {
    t_loopInThisThread = this;
  }

  wakeupChanenl_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChanenl_->enableReading();
}

EventLoop::~EventLoop() {
  wakeupChanenl_->disableAll();
  wakeupChanenl_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  looping_ = true;
  quit_ = false;

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
}

// useful?
void EventLoop::doPendingFunctors() {
  std::vector<callback_t<>> functors;
  callingPendingFunctors_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
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

size_t EventLoop::queueSize() /* const */ {
  std::lock_guard<std::mutex> lock(mutex_);
  return pendingFunctors_.size();
}

void EventLoop::runInLoop(callback_t<> callback) {
  if (isInLoopThread()) {
    callback();
  } else {
    queueInLoop(std::move(callback)); // why into queue
  }
}

void EventLoop::quit() {
  quit_ = true;
  if (isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    // LOG error
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    // LOG error
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