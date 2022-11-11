#include "muduo/util/Thread.h"
#include "muduo/util/CurrentThread.h"

#include <semaphore.h> // signal
#include <memory>

using namespace muduo;

std::atomic<int> Thread::numCreated_;

Thread::Thread(callback_t<> func, const std::string& n) : started_(false),
  joined_(false), tid_(0), func_(std::move(func)), name_(n) {
  setDefaultName();
}

Thread::~Thread() {
  // the father thread will lose the control of children thread
  // running lib is responsible for resource recovery
  if (started_ && !joined_) {
    thread_->detach();
  }
}

void Thread::start() {
  started_ = true;
  sem_t sem;
  sem_init(&sem, false, 0);
  // new a thread object, it keeps alive although start func returned
  thread_ = std::make_shared<std::thread>([&](){
    tid_ = CurrentThread::tid();
    sem_post(&sem);
    func_();
  });

  // to ensure tid_ has updated
  sem_wait(&sem);
}

void Thread::join() {
  joined_ = true;
  thread_->join();
}

void Thread::setDefaultName() {
  if (name_.empty()) {
    char buf[32] = {0};
    // what's the difference of snprintf and sprintf
    snprintf(buf, sizeof buf, "Thread%d", ++numCreated_);
    name_ = buf;
  }
}