#include "util/ring_buffer_wrapper.h"

#include "rtc_base/checks.h"
namespace core {
constexpr size_t kExponentialGrowthThreshold = 1024 * 10;
RingBufferWrapper::RingBufferWrapper(size_t capacity, bool auto_adjust_capacity,
                                     size_t max_buffer_capacity)
    : buff_handle_(WebRtc_CreateBuffer(capacity, 1)),
      auto_adjust_capacity_(auto_adjust_capacity) {
  RTC_DCHECK(buff_handle_) << "capacity: " << capacity;
  if (max_buffer_capacity < kMaxBufferSize) {
    max_buffer_capacity_ = max_buffer_capacity;
  }
}

RingBufferWrapper::~RingBufferWrapper() { WebRtc_FreeBuffer(buff_handle_); }

const void* RingBufferWrapper::BufferRead(void* data, size_t in_size,
                                          size_t* out_size) {
  if (nullptr == data || nullptr == out_size) {
    return nullptr;
  }
  void* data_ptr = nullptr;
  *out_size = WebRtc_ReadBuffer(buff_handle_, &data_ptr, data, in_size);
  return data_ptr;
}

size_t RingBufferWrapper::BufferRead(void* data, size_t size) {
  return WebRtc_ReadBuffer(buff_handle_, nullptr, data, size);
}

size_t RingBufferWrapper::BufferWrite(const void* data, size_t size) {
  size_t size_canbe_write = BufferCapacity() - BufferCurrentSize();
  if (size_canbe_write < size) {
    if (auto_adjust_capacity_) {
      AdjustCapacity(CalculatedExpansionCapacity(size));
      if (BufferCapacity() >= max_buffer_capacity_) {
        auto_adjust_capacity_ = false;
        if (BufferCapacity() - BufferCurrentSize() < size) {
          WebRtc_MoveReadPtr(
              buff_handle_,
              static_cast<int>(size - BufferCapacity() + BufferCurrentSize()));
        }
      }
    } else {
      // RTC_DCHECK(false);
      WebRtc_MoveReadPtr(
          buff_handle_,
          static_cast<int>(size -
                           size_canbe_write));  // The extra data is thrown away
    }
  }
  return WebRtc_WriteBuffer(buff_handle_, data, size);
}

void RingBufferWrapper::Clear() { return WebRtc_InitBuffer(buff_handle_); }

size_t RingBufferWrapper::BufferCurrentSize() {
  return WebRtc_available_read(buff_handle_);
}

size_t RingBufferWrapper::BufferCapacity() {
  return nullptr == buff_handle_
             ? 0
             : buff_handle_->element_count * buff_handle_->element_size;
}

int RingBufferWrapper::BufferSeek(int offset) {
  return nullptr == buff_handle_ ? 0 : WebRtc_MoveReadPtr(buff_handle_, offset);
}

void RingBufferWrapper::AdjustCapacity(size_t capacity) {
  if (capacity < BufferCurrentSize() || capacity == BufferCapacity()) {
    return;
  }
  RingBuffer* new_buff_handle = WebRtc_CreateBuffer(capacity, 1);
  size_t read_size = BufferCurrentSize();
  if (read_size > 0) {
    int read_size1 =
        static_cast<int>(buff_handle_->write_pos - buff_handle_->read_pos);
    if (read_size1 < 0) {
      read_size1 = static_cast<int>(buff_handle_->element_count -
                                    buff_handle_->read_pos);
    }
    WebRtc_WriteBuffer(new_buff_handle,
                       buff_handle_->data + buff_handle_->read_pos, read_size1);
    if (static_cast<int>(read_size) > read_size1) {
      WebRtc_WriteBuffer(new_buff_handle, buff_handle_->data,
                         read_size - read_size1);
    }
  }
  WebRtc_FreeBuffer(buff_handle_);
  buff_handle_ = new_buff_handle;
}

size_t RingBufferWrapper::CalculatedExpansionCapacity(size_t write_size) {
  size_t new_size = BufferCapacity();
  while (new_size - BufferCurrentSize() < write_size) {
    new_size += new_size > kExponentialGrowthThreshold
                    ? kExponentialGrowthThreshold
                    : new_size;
  }
  RTC_DCHECK(write_size <= max_buffer_capacity_);
  return std::min(new_size, max_buffer_capacity_);
}
}  // namespace core
