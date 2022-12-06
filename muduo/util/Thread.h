#ifndef MUDUO_UTIL_THREAD_H_
#define MUDUO_UTIL_THREAD_H_

#include "muduo/util/noncopyable.h"
#include "muduo/util/callback.h"

#include <atomic>
#include <thread>
#include <memory>

namespace muduo {

/*-------------------------------Thread Design GUID-----------------------------*\
 * class Thread will use std::thread to encapsulate thread operation further.
 */

class Thread : noncopyable {
 public:
  explicit Thread(callback_t<>, const std::string& name = "");
  // FIXME: make it movable in c++11
  // cann't copy but can move
  ~Thread();

  void start();
  void join();

  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }

 private:
  void setDefaultName();

  bool started_;
  bool joined_;
  std::shared_ptr<std::thread> thread_;
  callback_t<> threadFunc_;
  std::string name_;
  std::atomic<int> num_;
  pid_t tid_;

  static std::atomic<int> numCreated_;
};

} // namespace muduo

#endif // !MUDUO_UTIL_THREAD_H_