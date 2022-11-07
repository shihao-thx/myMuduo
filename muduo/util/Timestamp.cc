#include "muduo/util/Timestamp.h"

#include <time.h>

using namespace muduo;

Timestamp::Timestamp() : ms_(0) {}

Timestamp::Timestamp(int64_t ms) : ms_(ms) {}

Timestamp Timestamp::now() {
  return Timestamp(time(NULL));
}

std::string Timestamp::toString() const {
  char buf[128] = {0};
  tm* tm_time = localtime(&ms_);
  snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
  tm_time->tm_year + 1900,
  tm_time->tm_mon + 1,
  tm_time->tm_mday,
  tm_time->tm_hour,
  tm_time->tm_min,
  tm_time->tm_sec);
  return buf;
}