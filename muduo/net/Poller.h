#ifndef MUDUO_NET_POLLER_H_
#define MUDUO_NET_POLLER_H_

#include <unordered_map>
#include <vector>

#include "muduo/util/noncopyable.h"
#include "muduo/util/Timestamp.h"

namespace muduo {

namespace net {

class EventLoop;
class Channel;

/*------------------------------Poller Design GUID-----------------------------*\
 * Poller is a base class for IO multiplxing, which is a encapsulation of poll(2)
 * epoll(4). Take epoll(4) as the example, we register events we care to it and 
 * it will return the ready events to us.
 */

class Poller : noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop* loop);
  virtual ~Poller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannel) = 0;

  virtual void updateChannel(Channel* channel) = 0;

  virtual void removeChannel(Channel* channel) = 0;

  virtual bool hasChannel(Channel* channel) const;

  // import derived classes is not a good choice
  // so, its implementation will not define here.
  static Poller* newDefaultPoller(EventLoop* loop);

 protected:
  std::unordered_map<int, Channel*> channels_; // why have this?

 private:
  EventLoop* ownerLoop_;
};

} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_POLLER_H_