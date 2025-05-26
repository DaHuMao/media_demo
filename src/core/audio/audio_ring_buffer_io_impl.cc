#include "core/audio/audio_ring_buffer_io_impl.h"
#include "rtc_base/checks.h"
namespace core {
static size_t GetBufferCount(const AudioFormatInfo &info) {
  return info.IsPlanner() ? info.GetChannelsCount() : 1;
}
AudioRingBufferIoImpl::AudioRingBufferIoImpl(const AudioFormatInfo &info,
                                             size_t buffer_capacity,
                                             bool auto_expand_capacity)
    : AudioBufferCommonReader(GetBufferCount(info),
                              buffer_capacity / GetBufferCount(info),
                              auto_expand_capacity),
      is_auto_expand_capacity_(auto_expand_capacity),
      format_info_(info),
      buffer_useable_capacity_(buffer_capacity / GetBufferCount(info)) {
  if (!format_info_.ValidSampleRateAndChannelCheck()) {
    status_ = SourceStatus::kStatic;
  }
}

void AudioRingBufferIoImpl::ResetUseableCapacity(size_t size) {
  RTC_DCHECK(size % circular_buffer_vec_.size() == 0);
  std::lock_guard<std::mutex> lk(mutex_);
  auto buffer_useable_capacity_tmp = size / circular_buffer_vec_.size();
  if (buffer_useable_capacity_ < buffer_useable_capacity_tmp) {
    for (auto &circular_buffer : circular_buffer_vec_) {
      circular_buffer->AdjustCapacity(buffer_useable_capacity_tmp);
    }
  }
  buffer_useable_capacity_ = buffer_useable_capacity_tmp;
}
size_t AudioRingBufferIoImpl::BlockingRead(void *data_ptr, int size) {
  return ReadInternal(data_ptr, size, true);
}

size_t AudioRingBufferIoImpl::BlockingWrite(const void *data, int size) {
  return WriteInternal(data, size, true);
}

void AudioRingBufferIoImpl::CancelRead() {
  std::lock_guard<std::mutex> lk(mutex_);
  NotifyReader();
}

void AudioRingBufferIoImpl::CancelWrite() {
  std::lock_guard<std::mutex> lk(mutex_);
  NotifyWriter();
}

void AudioRingBufferIoImpl::CancelAll() {
  std::lock_guard<std::mutex> lk(mutex_);
  NotifyReader();
  NotifyWriter();
}

size_t AudioRingBufferIoImpl::Read(void *data_ptr, size_t data_size) {
  return ReadInternal(data_ptr, data_size, false);
}
int AudioRingBufferIoImpl::DiscardDataSizeInByte(size_t size) {
  std::lock_guard<std::mutex> lk(mutex_);
  int read_size = BaseDiscardDataSizeInByte(size);
  CheckIfNotifyWrite();
  return read_size;
}

size_t AudioRingBufferIoImpl::CurrentSize() {
  std::lock_guard<std::mutex> lk(mutex_);
  return BaseCurrentSize();
}

SourceStatus AudioRingBufferIoImpl::GetSourceStatus() const {
  return BaseGetSourceStatus();
}

int AudioRingBufferIoImpl::Reset() {
  std::lock_guard<std::mutex> lk(mutex_);
  NotifyReader();
  NotifyWriter();
  return BaseReset();
}

int AudioRingBufferIoImpl::FillZeroFront(util::MillisecondsClass time_ms) {
  std::lock_guard<std::mutex> lk(mutex_);
  return BaseFillZeroFront(format_info_.AudioMsToByteSize(time_ms));
}

void AudioRingBufferIoImpl::WriteCompletion() {
  std::lock_guard<std::mutex> lk(mutex_);
  if (SourceStatus::kStreaming == status_) {
    status_ = SourceStatus::kStatic;
  }
  CheckStatus();
}

const AudioFormatInfo &AudioRingBufferIoImpl::GetAudioFormatInfo() const {
  return format_info_;
}
const AudioFormatInfo &AudioRingBufferIoImpl::GetNeededAudioFormatInfo() const {
  return format_info_;
}

size_t AudioRingBufferIoImpl::Write(const void *data_ptr, size_t data_size) {
  return WriteInternal(data_ptr, data_size, false);
}

size_t AudioRingBufferIoImpl::FreeSpaceBeforeOverwriting() {
  std::lock_guard<std::mutex> lk(mutex_);
  return FreeSpaceBeforeOverwritingNoLock();
}

util::MillisecondsClass AudioRingBufferIoImpl::FreeSpaceBeforeOverwritingMs() {
  return format_info_.AudioByteSizeToMs(FreeSpaceBeforeOverwriting());
}

size_t AudioRingBufferIoImpl::FreeSpaceBeforeOverwritingNoLock() {
  if (SourceStatus::kStatic == status_) {
    return 0;
  }
  RTC_DCHECK_GE(buffer_useable_capacity_,
                circular_buffer_vec_[0]->BufferCurrentSize());
  int64_t can_write = (static_cast<int64_t>(buffer_useable_capacity_) -
                       circular_buffer_vec_[0]->BufferCurrentSize()) *
                      circular_buffer_vec_.size();
  can_write = should_seek_size_ > 0
                  ? can_write + should_seek_size_ * circular_buffer_vec_.size()
                  : can_write;
  return can_write > 0 ? can_write : 0;
}
size_t AudioRingBufferIoImpl::ReadInternal(void *data_ptr, size_t data_size,
                                           bool is_blocking) {
  std::unique_lock<std::mutex> lk(mutex_);
  if (is_blocking) {
    WaitingRead(data_size, lk);
  }
  size_t read_size = BaseRead(data_ptr, data_size);
  CheckIfNotifyWrite();
  return read_size;
}

size_t AudioRingBufferIoImpl::WriteInternal(const void *data_ptr,
                                            size_t data_size,
                                            bool is_blocking) {
  if (data_ptr == nullptr || data_size == 0) {
    return 0;
  }
  bool checks = data_size % circular_buffer_vec_.size() == 0;
  if (!checks) {
    RTC_DCHECK(false) << "data_size is not multiple of buffer count"
                      << " data size: " << data_size
                      << " buffer count: " << circular_buffer_vec_.size();
    return 0;
  }
  size_t should_write_size = data_size / circular_buffer_vec_.size();
  std::vector<const uint8_t *> data_vec;
  for (size_t i = 0; i < circular_buffer_vec_.size(); i++) {
    data_vec.push_back(reinterpret_cast<const uint8_t *>(data_ptr) +
                       i * should_write_size);
  }
  return WritePlanarInternal(data_vec, should_write_size, is_blocking);
}

void AudioRingBufferIoImpl::WaitingWrite(size_t should_write_size,
                                         std::unique_lock<std::mutex> &lk) {
  while (!cancel_write_ && status_ == SourceStatus::kStreaming &&
         FreeSpaceBeforeOverwritingNoLock() < should_write_size) {
    waiting_write_size_ = should_write_size;
    cv_writer_.wait(lk);
  }
  waiting_write_size_ = 0;
  cancel_write_ = false;
}

void AudioRingBufferIoImpl::WaitingRead(size_t should_read_size,
                                        std::unique_lock<std::mutex> &lk) {
  while (!cancel_read_ && status_ == SourceStatus::kStreaming &&
         CurrentSizeInternal() < should_read_size) {
    waiting_read_size_ = should_read_size;
    cv_reader_.wait(lk);
  }
  waiting_read_size_ = 0;
  cancel_read_ = false;
}

void AudioRingBufferIoImpl::CheckIfNotifyRead() {
  if (waiting_read_size_ > 0 && CurrentSizeInternal() >= waiting_read_size_) {
    cv_reader_.notify_one();
  }
}

void AudioRingBufferIoImpl::CheckIfNotifyWrite() {
  if (waiting_write_size_ > 0 &&
      FreeSpaceBeforeOverwritingNoLock() >= waiting_write_size_) {
    cv_writer_.notify_one();
  }
}

void AudioRingBufferIoImpl::NotifyReader() {
  if (waiting_read_size_ > 0) {
    cancel_read_ = true;
    cv_reader_.notify_one();
  }
}
void AudioRingBufferIoImpl::NotifyWriter() {
  if (waiting_write_size_ > 0) {
    cancel_write_ = true;
    cv_writer_.notify_one();
  }
}

size_t AudioRingBufferIoImpl::WritePlanar(
    const std::vector<const uint8_t *> &data, size_t size_byte_every_dim) {
  return WritePlanarInternal(data, size_byte_every_dim, false);
}

size_t AudioRingBufferIoImpl::WritePlanarInternal(
    const std::vector<const uint8_t *> &data, size_t data_size_one_dim,
    bool is_blocking) {
  if (data.size() != circular_buffer_vec_.size()) {
    RTC_DCHECK(false) << "data size is not equal to buffer count"
                      << " data size: " << data.size()
                      << " buffer count: " << circular_buffer_vec_.size();
    return 0;
  }
  size_t should_write_size = data_size_one_dim, data_seek_size = 0;
  std::unique_lock<std::mutex> lk(mutex_);
  if (status_ != SourceStatus::kStreaming) {
    return 0;
  }

  if (!is_auto_expand_capacity_) {
    if (data_size_one_dim > buffer_useable_capacity_) {
      RTC_DCHECK(false) << "data_size is larger than buffer capacity"
                        << " data size: " << data_size_one_dim
                        << " buffer capacity: " << buffer_useable_capacity_;
      should_write_size = buffer_useable_capacity_;
      data_seek_size = data_size_one_dim - buffer_useable_capacity_;
    }
    if (is_blocking) {
      WaitingWrite(should_write_size, lk);
    }
    if (should_write_size > FreeSpaceBeforeOverwritingNoLock()) {
      BaseDiscardDataSizeInByte(should_write_size -
                                FreeSpaceBeforeOverwritingNoLock());
    }
  }

  size_t should_seek_size = 0, write_size = 0;
  if (should_seek_size_ > 0) {
    should_seek_size =
        std::min(should_write_size, static_cast<size_t>(should_seek_size_));
    should_write_size -= should_seek_size;
    should_seek_size_ -= static_cast<int32_t>(should_seek_size);
  }
  for (size_t i = 0; i < circular_buffer_vec_.size(); i++) {
    auto ptr = data[i] + should_seek_size + data_seek_size;
    write_size = circular_buffer_vec_[i]->BufferWrite(ptr, should_write_size);
  }
  CheckIfNotifyRead();
  return (write_size + should_seek_size) * circular_buffer_vec_.size();
}

std::unique_ptr<AudioRingBufferIo> AudioRingBufferIo::Create(
    const core::AudioFormatInfo &audio_format, uint32_t size_in_byte) {
  return std::make_unique<AudioRingBufferIoImpl>(audio_format, size_in_byte,
                                                 false);
}

std::unique_ptr<AudioRingBufferIo> AudioRingBufferIo::CreateAutoExpandBuffer(
    const core::AudioFormatInfo &audio_format, uint32_t size_in_byte) {
  return std::make_unique<AudioRingBufferIoImpl>(audio_format, size_in_byte,
                                                 true);
}

}  // namespace core
