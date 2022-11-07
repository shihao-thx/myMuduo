#include "muduo/net/Poller.h"
#include "muduo/net/poller/EpollPoller.h"

using namespace muduo;
using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    return new EpollPoller(loop);
}