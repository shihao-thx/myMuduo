#include "src/net/Buffer.h"
#include "glog/logging.h"

#include <sys/uio.h>
#include <unistd.h>

using namespace muduo::net;


const size_t Buffer::kPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int* savedErrno) {
  char extraBuf[65536]; // 64kB
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extraBuf;
  vec[1].iov_len = sizeof extraBuf;
  // we want to read more than 64kB at one time with the help of extraBuf
  const int iovcnt = writable > sizeof extraBuf? 1 : 2;
  const ssize_t n = ::readv(fd, vec, iovcnt);

  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    writerIndex_+=n;
  } else {
    writerIndex_ = buffer_.size();
    append(extraBuf, n - writable);
  }
  return n;
}

ssize_t Buffer::writeFd(int fd) {
  ssize_t number;
  if ((number = ::write(fd, peek(), readableBytes())) < 0) {
    LOG(ERROR) << "Buffer::writeFd";
  }
  return number;
}