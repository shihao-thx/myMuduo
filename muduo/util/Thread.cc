#include "muduo/util/Thread.h"
#include "muduo/util/CurrentThread.h"

#include <semaphore.h> // signal
#include <memory>

using namespace muduo;

std::atomic<int> Thread::numCreated_; // initialized

Thread::Thread(callback_t<> func, const std::string& name) : started_(false),
    joined_(false), tid_(0), threadFunc_(std::move(func)), name_(name) {
  if (name_.empty()) {
  //   name_ = "Thread" + (++numCreated_); // hread, read
  //   name_ = "Thread";
  //   name_ += (++numCreated_); // Wrong! different type
    setDefaultName();
  }
}

Thread::~Thread() {
  // detach(): The father thread will lose the control of children thread.
  // running lib is responsible for resource recovery rather than father thread.
  if (started_ && !joined_) {
    thread_->detach();
  }
}

void Thread::start() {
  started_ = true;
  ::sem_t sem;
  // value argument 0 specifies the initial value for the semaphore.
  ::sem_init(&sem, false, 0);
  // new a thread object, it keeps alive although start func returned
  // std:: thrad constructor will be called
  thread_ = std::make_shared<std::thread>([&](){
    tid_ = CurrentThread::tid();
    // increments(unlocks) the semaphore pointed to by sem.  
    ::sem_post(&sem);
    threadFunc_();
  });

  // to ensure tid_ has updated
  sem_wait(&sem);
}

void Thread::join() {
  joined_ = true;
  thread_->join();
}

void Thread::setDefaultName() {
  char buf[32] = {0};
  snprintf(buf, sizeof buf, "Thread%d", ++numCreated_);
  name_ = buf;
}

// int main() {
//   callback_t<> c1;
//   Thread t1(c1);
//   Thread t2(c1);
//   return 0;
// }