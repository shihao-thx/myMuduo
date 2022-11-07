#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "muduo/net/poller/EpollPoller.h"
#include "muduo/net/Channel.h"

using namespace muduo;
using namespace muduo::net;

const int kNew = -1;     // channel hasn't added into poller
const int kAdded = 1;    // added
const int kDeleted = 2;  // deleted

EpollPoller::EpollPoller(EventLoop* loop) : Poller(loop), 
  epollfd_(::epoll_create(EPOLL_CLOEXEC)), events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    // TODO LOG
    std::cout << "Fatal" << std::endl;
  }
}

EpollPoller::~EpollPoller() { ::close(epollfd_); }

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
  // TODO: LOG
  // return happenning events in events_
  int eventsNum = ::epoll_wait(epollfd_, events_.data(), 
    static_cast<int>(events_.size()), timeoutMs);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if (eventsNum > 0) {
    // TODO: 
    fillActiveChannels(eventsNum, activeChannels);
    if (static_cast<size_t>(eventsNum) == events_.size()) {
      // the size of events_ may limit the eventNum,
      // scale the events_ and the limited events will be handled next time
      events_.resize(events_.size()*2);
    }
  } else if (eventsNum == 0) {
    // TODO
    std::cout << "timeout!" << std::endl;
  } else {
    if (savedErrno != EINTR) {
      errno = savedErrno;
      // TODO
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
  // TODO: assertInLoopthread
  const int index = channel->index();
  std::cout << "loginfo" << std::endl;
  int fd = channel->fd();
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      channels_[fd] = channel;
    } else {
      // assert the fd is already in the Poller
    }
    channel->setIndex(kAdded);
    update(EPOLL_CTL_ADD, channel); // why we should pass channel
  } else {    // want to change the type of the event for Channel 
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
  event.events - channel->events();
  event.data.ptr = channel;

  int fd = channel->fd();
  // TODO: log
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      // TODO: Err log
    } else {
      // Fatal log
    }
  }
}
