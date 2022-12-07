#ifndef MUDUO_NET_POLLER_EPOLLPOLLER_H_
#define MUDUO_NET_POLLER_EPOLLPOLLER_H_

#include "src/net/Poller.h"

namespace muduo {

namespace net {
   

class EpollPoller : public Poller {
 public:
  EpollPoller(EventLoop* loop);
  ~EpollPoller() override;

  // called in EventLoop
  Timestamp poll(int timeoutMs, ChannelList* activeChannel) override;

  void updateChannel(Channel* channel) override;

  void removeChannel(Channel* channel) override;
 
  private:
   static const int kInitEventListSize = 16;

   static const char* operationToString(int op);

   void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

   void update(int operation, Channel* channel);

   int epollfd_;
   std::vector<struct epoll_event> events_; // aims to do what? 

};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_POLLER_EPOLLPOLLER_H_