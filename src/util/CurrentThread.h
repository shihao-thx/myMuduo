#ifndef MUDUO_UTIL_CURRENTTHREAD_H_
#define MUDUO_UTIL_CURRENTTHREAD_H_

#include <unistd.h>
#include <sys/syscall.h>

namespace muduo {

namespace CurrentThread {

extern __thread int t_cachedTid;

void cacheTid();

inline int tid() {
  // t_cachedTid == 0 is likely to be false 
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

} // namespace CurrentThread

} // namespace muduo

#endif // !MUDUO_UTIL_CURRENTTHREAD_H_