#include "muduo/net/TcpConnection.h"
#include "muduo/net/Socket.h"
#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"
#include "glog/logging.h"
#include "muduo/util/Timestamp.h"

using namespace muduo::net;

EventLoop* muduo::net::check_loop_not_null(EventLoop* loop) {
  if (loop == nullptr) {
    LOG(FATAL) << "EventLoop is NULL";
  }
  return loop;
}
void muduo::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG(INFO) << conn->serverAddress().toIpPort() << " -> " << conn->clientAddress().toIpPort()
            << " is " << (conn->connected()? "UP" : "DOWN");
}

void muduo::net::defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, 
                                        Timestamp receiveTime) {
  buffer->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
  const InetAddress& serverAddr, const InetAddress& clientAddr) 
  : loop_(check_loop_not_null(loop)), name_(name), state_(kConnecting), 
    reading_(true), socket_(new Socket(sockfd)), channel_(new Channel(loop, sockfd)),
    serverAddr_(serverAddr), clientAddr_(clientAddr), highWaterMark_(64*1024*1024) {
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  DLOG(INFO) << "TcpConnection::constructor[" << name_ << "] at " << this 
             << " fd = " << sockfd;
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  DLOG(INFO) << "TcpConnection::deconstructor[" << name_ << "] at " << this 
             << " fd = " << channel_->fd() << "state = " << stateTostring();
}

bool TcpConnection::getTcpInfo(tcp_info* ptcp) const {
  return socket_->getTcpInfo(ptcp);
}

std::string TcpConnection::getTcpInfoString() const {
  char buf[1024];
  buf[0] = '\0';
  socket_->getTcpInfoString(buf, sizeof buf);
  return buf; // std::string s(buf);
}

void TcpConnection::send(const std::string& message) {
  if (connected_.load()) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      // value passed implicitly in std::bind
      loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
    }
  }
}

void TcpConnection::sendInLoop(const std::string& message) {
  ssize_t nwrote = 0;
  size_t len = message.size();
  size_t remaining = len;
  bool faultError = false;
  if (!connected_.load()) {
    LOG(WARNING) << "disconneted! give up writing"; 
    return;
  }
  // if nothing in outputBuffer, write directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    if ((nwrote = ::write(channel_->fd(), message.c_str(), len)) >= 0) {
      remaining = len - nwrote;
      // all sended
      if (remaining == 0 && writeCompleteCallback_) {
        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      // EWOULDBLOCK means no data and directly return due to unblocking 
      if (errno != EWOULDBLOCK) {
        LOG(FATAL) << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) {
          faultError = true;
        }
      }
    }
  }
  // if something is in Buffer
  if (!faultError && remaining > 0) {
    size_t oldLen = outputBuffer_.readableBytes();
    // oldLen > highWaterMark_ means highWaterMarkCallback_ has called
    if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_
        && highWaterMark_) {
      loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(),
        oldLen + remaining));
    }   
    outputBuffer_.append(message.c_str() + nwrote, remaining);
    // can write, because something in buffer
    if (!channel_->isWriting()) { 
      channel_->enableWriting(); // set writing
    }
  }
}

void TcpConnection::shutdown() {
  if (connected_.load()) {
    setState(kDisconnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop() {
  // judge if data in Buffer exists
  if (!channel_->isWriting()) {
    socket_->shutdownWrite();
  }
}

void TcpConnection::connectEstablished() {
  connected_.store(true);
  channel_->tie(shared_from_this());
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  if (connected_.load()) {
    connected_.store(false);
    channel_->disableAll();
    // why?
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG(ERROR) << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite() {
  if (channel_->isWriting()) {
    ssize_t n = outputBuffer_.writeFd(channel_->fd());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (writeCompleteCallback_) {
          loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == kDisconnecting) {
          shutdownInLoop();
        }
      }
    } else {
      LOG(ERROR) << "TcpConnection::handleWrite";
    }
  } else {
    DLOG(INFO) << "Connection fd = " << channel_->fd() << " is down, no more writing";
  }
}

void TcpConnection::handleClose() {
  DLOG(INFO) << "fd = " << channel_->fd() << " state = " << stateTostring();
  connected_.store(false);
  channel_->disableAll();
  // What does these mean? ensure TcpConnection ptr released?
  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  closeCallback_(guardThis);
}

void TcpConnection::handleError() {
  int optval;
  int err = 0;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);
  if (::getsockopt(socket_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    err = errno;
  } else {
    err = optval;
  }
  LOG(ERROR) << "Tcpconnection::handleError [" << name_ << "] - SO_ERROR = " << err
             << " " << strerror(err);
}

void TcpConnection::forceCloseInLoop() {}

const char* TcpConnection::stateTostring() {
  switch (state_) {
    case kConnecting:
      return "kConnecting";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      if (connected_.load())
        return "kConnected";
      return "Disconnected";
  }
}
void TcpConnection::startReadInLoop() {}
void TcpConnection::stopReadInLoop() {}

// int main() {
//   InetAddress a1(10);
//   InetAddress a2(20);
//   EventLoop loop;
//   TcpConnection c(&loop, "a", 1, a1, a2);
//   struct tcp_info t1;
//   c.getTcpInfo(&t1);
//   return 0;
// }