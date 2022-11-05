#ifndef MUDUO_UTIL_CALLBACK_H_
#define MUDUO_UTIL_CALLBACK_H_

#include <functional>

namespace muduo {

template<class...  Args>
using callback_t = std::function<void(Args...)>;

} // namespace muduo

#endif // !MUDUO_UTIL_CALLBACK_H_
