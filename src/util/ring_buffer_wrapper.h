#ifndef UTIL_RING_BUFFER_WRAPPER_H_
#define UTIL_RING_BUFFER_WRAPPER_H_
#include <cstddef>

#include "webrtc/common_audio/ring_buffer.h"
namespace core {
//----------------------  CacheBuffer ----------------------------------//
constexpr size_t kMaxBufferSize = 50 * 1024 * 1024;
class RingBufferWrapper {
 public:
  RingBufferWrapper(const RingBufferWrapper&) = delete;
  RingBufferWrapper(RingBufferWrapper&&) = delete;
  // If expandable_ is true, the buffer will automatically expand when it is
  // full, otherwise, the buffer will overwrite the past data when it is full
  RingBufferWrapper(size_t capacity, bool auto_expand_capacity = false,
                    size_t max_buffer_capacity = kMaxBufferSize);
  virtual ~RingBufferWrapper();
  const void* BufferRead(void* data, size_t in_size, size_t* out_size);
  size_t BufferRead(void* data, size_t size);
  size_t BufferWrite(const void* data, size_t size);
  void Clear();
  size_t BufferCurrentSize();
  size_t BufferCapacity();
  int BufferSeek(int offset);
  void AdjustCapacity(size_t capacity);

 private:
  size_t CalculatedExpansionCapacity(size_t write_size);

 private:
  RingBuffer* buff_handle_ = nullptr;
  // If buffer is in expandable mode, exceeding this capacity will automatically
  // turn into non-expandable mode
  bool auto_adjust_capacity_ = false;
  size_t max_buffer_capacity_ = kMaxBufferSize;
};

}  // namespace core
#endif  // UTIL_RING_BUFFER_WRAPPER_H_
