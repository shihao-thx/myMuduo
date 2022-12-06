#ifndef MUDUO_UTIL_CALLBACK_H_
#define MUDUO_UTIL_CALLBACK_H_

#include <functional>
#include <memory>

namespace muduo {

class Timestamp;

template<class...  Args>
using callback_t = std::function<void(Args...)>;

namespace net {  

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class EventLoop;
EventLoop* check_loop_not_null(EventLoop* loop);

class Buffer;
void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);

}

} // namespace muduo

#endif // !MUDUO_UTIL_CALLBACK_H_
