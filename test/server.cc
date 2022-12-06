#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include <functional>

#include "glog/logging.h"

class EchoServer {
 public:
  EchoServer(muduo::net::EventLoop* loop,
    const muduo::net::InetAddress& listenAddr) 
    : server_(loop, listenAddr, "EchoServer") {
    server_.setConnectionCallback(std::bind(&EchoServer::onConnection, 
      this, std::placeholders::_1)); 
    server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, 
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void start() {
    server_.start();
  }
 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn) {
    LOG(INFO) << "EchoServer - " << conn->clientAddress().toIpPort() << " -> "
              << conn->serverAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn, 
    muduo::net::Buffer* buf, muduo::Timestamp time) {
    std::string msg(buf->retriveAllAsString());
    LOG(INFO) << conn->name() << " echo " << msg.size() << " bytes, "
              << "data received at " << time.toString();
    conn->send(msg);
  }
  muduo::net::TcpServer server_;
};

int main() {
  //google::InitGoogleLogging("myMuduo");
  LOG(INFO) << "pid = " << getpid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress serverAddr(8080);
  EchoServer server(&loop, serverAddr);
  server.start();
  loop.loop();
  return 0;
}