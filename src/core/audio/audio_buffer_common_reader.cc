#include "core/audio/audio_buffer_common_reader.h"

#include <algorithm>

#include "rtc_base/checks.h"
namespace core {
AudioBufferCommonReader::AudioBufferCommonReader(size_t buffer_count,
                                                 size_t buffer_capacity,
                                                 bool auto_expand_capacity)
    : status_(SourceStatus::kStreaming) {
  circular_buffer_vec_ =
      std::vector<std::unique_ptr<core::RingBufferWrapper>>(buffer_count);
  for (std::unique_ptr<core::RingBufferWrapper>& ptr : circular_buffer_vec_) {
    ptr.reset(
        new core::RingBufferWrapper(buffer_capacity, auto_expand_capacity));
  }
}
AudioBufferCommonReader::~AudioBufferCommonReader() {}

size_t AudioBufferCommonReader::BaseRead(void* data_ptr, size_t data_size) {
  if (data_ptr == nullptr || data_size == 0) {
    return 0;
  }
  bool checks = data_size % circular_buffer_vec_.size() == 0;
  RTC_DCHECK(checks);
  if (!checks) {
    return 0;
  }
  size_t should_read_size = data_size / circular_buffer_vec_.size();
  size_t should_seek_size = 0;
  if (should_seek_size_ < 0) {
    should_seek_size =
        std::min(should_read_size, static_cast<size_t>(-should_seek_size_));
    should_read_size -= should_seek_size;
    should_seek_size_ += static_cast<int32_t>(should_seek_size);
  }
  should_read_size = std::min(CurrentSizeInternal(), should_read_size);
  ReadForPlanner(static_cast<int32_t>(should_seek_size),
                 static_cast<int32_t>(should_read_size), data_ptr);
  CheckStatus();
  return (should_seek_size + should_read_size) * circular_buffer_vec_.size();
}

int AudioBufferCommonReader::BaseDiscardDataSizeInByte(size_t byte_size) {
  if ((byte_size % circular_buffer_vec_.size()) != 0) {
    RTC_DCHECK(false);
    return -1;
  }

  should_seek_size_ +=
      static_cast<int32_t>(byte_size / circular_buffer_vec_.size());
  if (should_seek_size_ > 0) {
    int size_can_seek = std::min(static_cast<int32_t>(GetSingleBufferMinSize()),
                                 should_seek_size_);
    should_seek_size_ -= size_can_seek;
    for (auto& circular_buffer : circular_buffer_vec_) {
      circular_buffer->BufferSeek(size_can_seek);
    }
    CheckStatus();
  }
  return static_cast<int32_t>(byte_size);
}

size_t AudioBufferCommonReader::BaseCurrentSize() const {
  return CurrentSizeInternal();
}

SourceStatus AudioBufferCommonReader::BaseGetSourceStatus() const {
  return status_;
}

int AudioBufferCommonReader::BaseFillZeroFront(size_t data_size) {
  if (status_ == SourceStatus::kError ||
      0 != (data_size % circular_buffer_vec_.size())) {
    return -1;
  }
  should_seek_size_ =
      0 - static_cast<int32_t>(data_size / circular_buffer_vec_.size());
  return 0;
}

int AudioBufferCommonReader::BaseReset() {
  for (auto& circular_buffer : circular_buffer_vec_) {
    circular_buffer->Clear();
  }
  should_seek_size_ = 0;
  status_ = SourceStatus::kStreaming;
  return 0;
}

size_t AudioBufferCommonReader::CurrentSizeInternal() const {
  return static_cast<int>(GetSingleBufferMinSize()) > should_seek_size_
             ? (GetSingleBufferMinSize() - should_seek_size_) *
                   circular_buffer_vec_.size()
             : 0;
}

size_t AudioBufferCommonReader::GetSingleBufferMinSize() const {
  size_t min_size = circular_buffer_vec_[0]->BufferCurrentSize();
  for (size_t i = 1; i < circular_buffer_vec_.size(); ++i) {
    min_size = std::min(min_size, circular_buffer_vec_[1]->BufferCurrentSize());
  }
  return min_size;
}

void AudioBufferCommonReader::ReadForInterweave(int should_seek_size,
                                                int should_read_size,
                                                void* data_ptr) {
  RTC_CHECK(false) << "Not support";
}

void AudioBufferCommonReader::ReadForPlanner(int should_seek_size,
                                             int should_read_size,
                                             void* data_ptr) {
  char* read_ptr = reinterpret_cast<char*>(data_ptr);
  for (auto& circular_buffer : circular_buffer_vec_) {
    if (should_seek_size > 0) {
      memset(read_ptr, 0, should_seek_size);
      read_ptr += should_seek_size;
    }
    circular_buffer->BufferRead(reinterpret_cast<void*>(read_ptr),
                                should_read_size);
    read_ptr += should_read_size;
  }
}

}  // namespace core
