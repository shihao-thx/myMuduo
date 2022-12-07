#include "src/net/Poller.h"
#include "src/net/Channel.h"

using namespace muduo;
using namespace muduo::net;

Poller::Poller(EventLoop* loop) : ownerLoop_(loop) {}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const {
  auto it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}