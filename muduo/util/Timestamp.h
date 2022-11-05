#ifndef MUDUO_UTIL_TIMESTAMP_H_
#define MUDUO_UTIL_TIMESTAMP_H_   

#include <iostream>
#include <string>

namespace muduo {

class Timestamp {
 public:
  Timestamp();
  explicit Timestamp(int64_t ms);
  static Timestamp now();
  std::string toString() const;
 
 private:
  int64_t ms_;
};

} // namespace muduo

#endif // !MUDUO_UTIL_TIMESTAMP_H_