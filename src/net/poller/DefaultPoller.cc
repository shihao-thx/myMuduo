#include "src/net/Poller.h"
#include "src/net/poller/EpollPoller.h"

using namespace muduo;
using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    return new EpollPoller(loop);
}