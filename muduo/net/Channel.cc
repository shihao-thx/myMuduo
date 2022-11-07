#include <poll.h>

#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"
#include "muduo/util/Timestamp.h"

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

// useful
void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::handleEvent(Timestamp receiveTime) {
  if (revents_ & POLLNVAL) {
    // TODO: class Logger and self-defined iostream 
    //std::cout << "WARNING" << std::endl; 
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_) readCallback_(receiveTime);
  }
  if (revents_ & POLLOUT) {
    if (writeCallback_) writeCallback_();
  }

}

void Channel::remove() {
  addedToLoop_ = false;
  loop_->removeChannel(this);
}