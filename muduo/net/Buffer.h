#ifndef MUDUO_NET_BUFFER_H_
#define MUDUO_NET_BUFFER_H_

#include <string>
#include <vector>

namespace muduo {

namespace net {

/*
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 */
class Buffer {
 public:
  static const size_t kPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t kInitialSize = kInitialSize) : 
    buffer_(kPrepend + kInitialSize), readerIndex_(kPrepend), 
    writerIndex_(kPrepend) {}

  size_t readableBytes() const { return writerIndex_ - readerIndex_; }
  size_t writableBytes() const { return buffer_.size() - writerIndex_; }
  size_t prePendableBytes() const { return readerIndex_; }
  const char* peek() { return begin() + readerIndex_; }

  // len means how long has already read from buffer
  // after reading, need to retrieve the buffer
  void retrieve(size_t len) {
    if (len < readableBytes()) {
      readerIndex_ += len;
    } else { 
      retrieveAll();
    }
  }

  void retrieveAll() {
    readerIndex_ = kPrepend;
    writerIndex_ = kPrepend;
  }

  std::string retrieveAsString(size_t len) {
    std::string result(peek(), len); 
    retrieve(len);
    return result;
  }

  std::string retriveAllAsString() {
    return retrieveAsString(readableBytes());
  }

  void append(const char* data, size_t len) {
    ensureWritableBytes(len);
    // can we aviod copying? std::string_view in C++17
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
  }

  void ensureWritableBytes(size_t len) {
    if (len > writableBytes()) {
      makeSpace(len);
    }
  }

  char* beginWrite() { return begin() + writerIndex_; }
  void hasWritten(size_t len) { writerIndex_ += len; }

  ssize_t readFd(int fd, int* savedErrno);
  ssize_t writeFd(int fd);

 private:
  char* begin() { return &*buffer_.begin(); }
  
  void makeSpace(size_t len) {  
    // good design
    if (len + kPrepend > writableBytes() + prePendableBytes()) {
      buffer_.resize(writerIndex_ + len);
    } else {
      // compress more space
      size_t readable = readableBytes();
      std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kPrepend);
      readerIndex_ = kPrepend;
      writerIndex_ = readerIndex_ + readable;
    }
  }

  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
};
} // namespace net

} // namespace muduo

#endif // !MUDUO_NET_BUFFER_H_
