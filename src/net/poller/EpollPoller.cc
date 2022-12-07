#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "src/net/poller/EpollPoller.h"
#include "src/net/Channel.h"
#include "src/util/Timestamp.h"
#include "glog/logging.h"

using namespace muduo;
using namespace muduo::net;

const int kNew = -1;     // channel hasn't added into poller
const int kAdded = 1;    // added
const int kDeleted = 2;  // deleted

EpollPoller::EpollPoller(EventLoop* loop) : Poller(loop), 
  epollfd_(::epoll_create(EPOLL_CLOEXEC)), events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG(FATAL) << "EPollPoller::EpollPoller";
  }
}

EpollPoller::~EpollPoller() { ::close(epollfd_); }

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
  DLOG(INFO) << "fd total count " << channels_.size();
  // return happenning events in events_
  // The memory area pointed to by events will contain the events that will be 
  // available for the caller.  Up to maxevents are returned by epoll_wait().  
  // The maxevents argument must be greater than zero. we initialize maxevents to 16
  int eventsNum = ::epoll_wait(epollfd_, events_.data(), 
    static_cast<int>(events_.size()), timeoutMs);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if (eventsNum > 0) {
    DLOG(INFO) << eventsNum << " events happened";
    fillActiveChannels(eventsNum, activeChannels);
    if (static_cast<size_t>(eventsNum) == events_.size()) {
      // the size of events_ may limit the eventNum,
      // scale the events_ and the limited events will be handled next time
      events_.resize(events_.size() * 2);
    }
  } else if (eventsNum == 0) {
    DLOG(INFO) << "nothing happened";
  } else {
    if (savedErrno != EINTR) {
      errno = savedErrno;
      LOG(ERROR) << "EPollPoller::poll()";
    }
  }
  return now;
}

void EpollPoller::fillActiveChannels(int eventsNum, ChannelList* activeChannels) const {
  for (int i = 0; i < eventsNum; ++i) {
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->setRevents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

void EpollPoller::updateChannel(Channel* channel) {
  const int index = channel->index();
  DLOG(INFO) << "updateChannel:" << " fd = " << channel->fd() << " events =" 
             << channel->eventsToString() << " type = " << channel->indexToString();
  int fd = channel->fd();
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      channels_[fd] = channel;
    } else {
      // assert the fd is already in the Poller
    }
    channel->setIndex(kAdded);
    update(EPOLL_CTL_ADD, channel); // why we should pass channel
  } else { // want to change the type of the event for Channel 
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EpollPoller::removeChannel(Channel* channel) {
  int fd = channel->fd();
  channels_.erase(fd);

  int index = channel->index();
  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

void EpollPoller::update(int operation, Channel* channel) {
  struct epoll_event event;
  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();

  DLOG(INFO) << "epoll_ctl op = " << operationToString(operation) << " fd = "
        << fd << " events =" << channel->eventsToString();
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG(ERROR) << "epoll_ctl op = " << operationToString(operation) << "fd = "
                 << fd;
    } else {
      LOG(ERROR) << "epoll_ctl op = " << operationToString(operation) << "fd = "
                 << fd;
    }
  }
}

const char* EpollPoller::operationToString(int op) {
  switch (op) {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    return "Unknown Operation";
  }
}
