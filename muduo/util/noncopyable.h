#ifndef MUDUO_UTIL_NONCOPYABLE_H_
#define MUDUO_UTIL_NONCOPYABLE_H_

namespace muduo {

class noncopyable {
public:
  noncopyable(const noncopyable&) = delete;
  // returning void doesn't matter, because we delete it.
  // noncopyable& operator=(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

protected:
  noncopyable() = default;
  ~noncopyable() = default;  
};
  
} // namespace muduo

#endif // !MUDUO_UTIL_NONCOPYABLE_H_ 
