#include <poll.h>

#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"
#include "muduo/util/Timestamp.h"
#include "glog/logging.h"

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) 
  : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false),
  eventHandling_(false), addedToLoop_(false) {}

Channel::~Channel() {}

void Channel::update() {
  addedToLoop_ = true;
  loop_->updateChannel(this);
} 

void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::handleEvent(Timestamp receiveTime) {
  eventHandling_ = true;
  if (revents_ & POLLNVAL) {
    // which means epfd is not a valid file descriptor.
    LOG(WARNING) << "fd = " << fd_ << " Channel:handleEvent() POLLNVAL";
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_) errorCallback_();
  }
  // POLLRDHUP means connection closed
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_) readCallback_(receiveTime);
  }
  // can write
  if (revents_ & POLLOUT) {
    if (writeCallback_) writeCallback_();
  }
  eventHandling_ = false;
}

void Channel::remove() {
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

std::string Channel::eventsToString() {
  std::string strevents;
  if (events_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    strevents += " readEvent";
  }
  if (events_ & POLLOUT) {
    strevents += " writeEvent";
  }
  return strevents;
}

const char* Channel::indexToString() {
  switch (index_) {
  case -1:
    return "kNew";
  case 1:
    return "kAdded";
  case 2:
    return "kDeleted";
  }
}