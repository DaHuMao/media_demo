#include "core/audio/audio_frame.h"

#include <cstring>

#include "rtc_base/checks.h"
#include "util/time_to_class.h"
namespace core {

static std::vector<uint8_t *> ToPlanarData(
    uint8_t *data, size_t size, const AudioFormatInfo &audio_format) {
  std::vector<uint8_t *> planar_data;
  if (audio_format.IsPlanner()) {
    auto one_channel_size = size / audio_format.GetChannelsCount();
    for (int i = 0; i < audio_format.GetChannelsCount(); i++) {
      planar_data.push_back(data + i * one_channel_size);
    }
  } else {
    planar_data.push_back(data);
  }
  return planar_data;
}

static std::vector<const uint8_t *> ToPlanarData(
    const uint8_t *data, size_t size, const AudioFormatInfo &audio_format) {
  auto vec = ToPlanarData(const_cast<uint8_t *>(data), size, audio_format);
  return std::vector<const uint8_t *>(vec.begin(), vec.end());
}

AudioFrameMaybePlanarView::AudioFrameMaybePlanarView(
    const std::vector<const uint8_t *> &data, size_t one_dim_size,
    const AudioFormatInfo& info) :
      const_data_arr_(data),
      audio_format_(info) {
  RTC_DCHECK(0 != one_dim_size && !data.empty());
  RTC_DCHECK_EQ(data.size(), info.GetChannelsCount());
  const_data_ = const_data_arr_.data()[0];
  byte_size_ = one_dim_size * info.GetChannelsCount();
  audio_format_ptr_ = &audio_format_;
}

AudioFrameMaybePlanarView::AudioFrameMaybePlanarView(const AudioFrameMaybePlanarView &src):
  const_data_arr_(src.const_data_arr_),
  audio_format_(src.audio_format_) {
  const_data_ = const_data_arr_.data()[0];
  byte_size_ = src.byte_size_;
  audio_format_ptr_ = &audio_format_;
}

AudioFrameMaybePlanarView::AudioFrameMaybePlanarView(
    AudioFrameMaybePlanarView &&src) noexcept
    : const_data_arr_(std::move(src.const_data_arr_)),
      audio_format_(src.audio_format_) {
  const_data_ = const_data_arr_.data()[0];
  byte_size_ = src.byte_size_;
  audio_format_ptr_ = &audio_format_;
}

AudioFormatSizeView::AudioFormatSizeView(size_t byte_size,
                                         const AudioFormatInfo *info)
    : byte_size_(byte_size), audio_format_ptr_(info) {
  RTC_DCHECK(nullptr != audio_format_ptr_)
      << "AudioFormatSizeView invalid param: "
      << "size: " << byte_size << " audio_format_ptr_: " << audio_format_ptr_;
}

AudioFrameLiteView::AudioFrameLiteView(const void *data, size_t size,
                                       const AudioFormatInfo *info)
    : AudioFormatSizeView(size, info), const_data_(data) {
  RTC_DCHECK(0 != size && nullptr != data);
}

size_t AudioFrameLiteView::CopyTo(void *data, size_t data_capacity) const {
  RTC_DCHECK(data_capacity >= byte_size_)
      << "data_capacity: " << data_capacity
      << " is less than size_: " << byte_size_;
  size_t copy_size = data_capacity < byte_size_ ? data_capacity : byte_size_;
  memcpy(data, const_data_, copy_size);
  return copy_size;
}

std::vector<const uint8_t *> AudioFrameLiteView::PlanarData() const {
  return ToPlanarData(reinterpret_cast<const uint8_t *>(const_data_),

                      byte_size_, *audio_format_ptr_);
}

AudioFrameLiteDelegate::AudioFrameLiteDelegate(
    uint8_t *data, size_t readable_data_size, size_t capacity_in_byte,
    const AudioFormatInfo &audio_format)
    : data_(data),
      capacity_in_byte_(capacity_in_byte),
      audio_format_(audio_format) {
  RTC_DCHECK(audio_format_.ValidPcmCheck())
      << " audio_format: " << audio_format_.ToString();
  if (readable_data_size > capacity_in_byte) {
    RTC_DCHECK(false) << "readable_data_size: " << readable_data_size
                      << " is greater than capacity_in_byte: "
                      << capacity_in_byte;
    readable_data_size = capacity_in_byte;
  }
  byte_size_ = readable_data_size;
  const_data_ = data_;
  audio_format_ptr_ = &audio_format_;
}
AudioFrameLiteDelegate::AudioFrameLiteDelegate(
    uint8_t *data, util::MillisecondsClass readable_data_size,
    size_t capacity_in_byte, const AudioFormatInfo &audio_format)
    : AudioFrameLiteDelegate(data,
                             audio_format.AudioMsToByteSize(readable_data_size),
                             capacity_in_byte, audio_format) {}

AudioFrameLiteDelegate::AudioFrameLiteDelegate(uint8_t *data,
                                               size_t readable_data_size,
                                               size_t capacity_in_byte,
                                               int sample_rate_hz,
                                               int num_channel)
    : AudioFrameLiteDelegate(data, readable_data_size, capacity_in_byte,
                             AudioFormatInfo(sample_rate_hz, num_channel)) {}

AudioFrameLiteDelegate::AudioFrameLiteDelegate(
    const AudioFormatInfo &audio_format)
    : audio_format_(audio_format) {
  RTC_DCHECK(audio_format_.ValidPcmCheck())
      << " audio_format: " << audio_format_.ToString();
  audio_format_ptr_ = &audio_format_;
}

AudioFrameLiteDelegate::AudioFrameLiteDelegate(
    AudioFrameLiteDelegate &&src) noexcept
    : data_(src.data_),
      capacity_in_byte_(src.capacity_in_byte_),
      audio_format_(src.audio_format_) {
  src.data_ = nullptr;
  src.capacity_in_byte_ = 0;
  audio_format_ptr_ = &audio_format_;
}

AudioFrameLiteDelegate::AudioFrameLiteDelegate(
    const AudioFrameLiteDelegate &src) noexcept
    : AudioFrameLiteDelegate(src.data_, src.byte_size_, src.capacity_in_byte_,
                             src.audio_format_) {}

std::vector<uint8_t *> AudioFrameLiteDelegate::MutablePlanarData() {
  return ToPlanarData(data_, byte_size_, audio_format_);
}

size_t AudioFrameLiteDelegate::CopyFrom(const void *data, size_t data_size) {
  RTC_DCHECK(data_size <= capacity_in_byte_)
      << "data_size: " << data_size
      << " is greater than capacity_in_byte_: " << capacity_in_byte_;
  size_t copy_size =
      capacity_in_byte_ < data_size ? capacity_in_byte_ : data_size;
  memcpy(data_, data, copy_size);
  ResetReadableSizeInByte(copy_size);
  return copy_size;
}

size_t AudioFrameLiteDelegate::CopyFrom(const AudioFrameLiteView &audio_frame) {
  audio_format_ = audio_frame.AudioFormat();
  return CopyFrom(audio_frame.Data(), audio_frame.ByteSize());
}

size_t AudioFrameLiteDelegate::CopyFrom(
    const AudioFrameMaybePlanarView &audio_frame) {
  audio_format_ = audio_frame.AudioFormat();
  RTC_DCHECK_LE(audio_frame.ByteSize(), capacity_in_byte_);
  size_t copy_size = capacity_in_byte_ < audio_frame.ByteSize()
                         ? capacity_in_byte_
                         : audio_frame.ByteSize();
  auto planar_data = audio_frame.PlanarData();
  uint8_t *ptr = data_;
  size_t copy_size_per_channel = copy_size / audio_format_.GetChannelsCount();
  for (size_t i = 0; i < planar_data.size(); i++) {
    memcpy(ptr, planar_data[i], copy_size_per_channel);
    ptr += copy_size_per_channel;
  }
  ResetReadableSizeInByte(copy_size);
  return copy_size;
}

void AudioFrameLiteDelegate::ResetReadableSize(util::MillisecondsClass ms) {
  ResetReadableSizeInByte(audio_format_.AudioMsToByteSize(ms));
}

void AudioFrameLiteDelegate::ResetReadableSizeInByte(size_t byte_size) {
  if (byte_size > capacity_in_byte_) {
    RTC_DCHECK(false) << " size: " << byte_size
                      << " is greater than capacity_in_byte_: "
                      << capacity_in_byte_;
    return;
  }
  RTC_DCHECK((byte_size % AudioFormat().ByteSizePerFrame()) == 0)
      << "unexpact size: " << byte_size << " FrameSize " << FrameSize();
  byte_size_ = byte_size;
}

size_t AudioFrameLiteDelegate::Append(const void *data, size_t data_size) {
  size_t free_size = FreeSizeInByte();
  size_t copy_size = free_size < data_size ? free_size : data_size;
  memcpy(data_ + byte_size_, data, copy_size);
  ResetReadableSizeInByte(byte_size_ + copy_size);
  return copy_size;
}

AudioFrameLite::AudioFrameLite(const AudioFormatInfo &info)
    : AudioFrameLiteDelegate(info) {}

AudioFrameLite::AudioFrameLite(const AudioFormatInfo &info,
                               size_t capacity_in_byte,
                               size_t readable_size_in_byte)
    : AudioFrameLiteDelegate(info) {
  RTC_DCHECK(static_cast<size_t>(0) != capacity_in_byte)
      << "AudioFrameLite invalid param, capacity_in_byte is 0 ";
  RTC_DCHECK_GE(capacity_in_byte, readable_size_in_byte);
  ExpandCapacityIfNeed(capacity_in_byte);
  byte_size_ = readable_size_in_byte;
}

AudioFrameLite::AudioFrameLite(util::MillisecondsClass capacity_in_ms,
                               const AudioFormatInfo &audio_format)
    : AudioFrameLite(audio_format,
                     audio_format.AudioMsToByteSize(capacity_in_ms), 0) {}

AudioFrameLite::AudioFrameLite(util::MillisecondsClass capacity_in_ms,
                               int sample_rate_hz, int num_channel)
    : AudioFrameLite(capacity_in_ms,
                     AudioFormatInfo(sample_rate_hz, num_channel)) {}

AudioFrameLite::AudioFrameLite(const AudioFormatInfo &info,
                               util::MillisecondsClass capacity_in_ms,
                               util::MillisecondsClass readable_size_in_ms)
    : AudioFrameLite(info, info.AudioMsToByteSize(capacity_in_ms),
                     info.AudioMsToByteSize(readable_size_in_ms)) {}
AudioFrameLite::AudioFrameLite(AudioFrameLite &&src) {
  data_ = src.data_;
  capacity_in_byte_ = src.capacity_in_byte_;
  audio_format_ = src.audio_format_;
  src.data_ = nullptr;
  src.capacity_in_byte_ = 0;
}

AudioFrameLite &AudioFrameLite::operator=(AudioFrameLite &&src) {
  if (this != &src) {
    if (nullptr != data_) {
      delete[] data_;
    }
    data_ = src.data_;
    capacity_in_byte_ = src.capacity_in_byte_;
    audio_format_ = src.audio_format_;
    src.data_ = nullptr;
    src.capacity_in_byte_ = 0;
  }
  return *this;
}

AudioFrameLite::~AudioFrameLite() {
  if (nullptr != data_) {
    delete[] data_;
  }
}

void AudioFrameLite::ExpandCapacityIfNeed(size_t capacity_in_byte) {
  if (capacity_in_byte > capacity_in_byte_) {
    uint8_t *new_data = new uint8_t[capacity_in_byte];
    if (byte_size_ > 0) {
      memcpy(new_data, data_, byte_size_);
    }
    if (nullptr != data_) {
      delete[] data_;
    }
    data_ = new_data;
    capacity_in_byte_ = capacity_in_byte;
    const_data_ = data_;
  }
}

/*void AudioFrameLite::Reset(size_t capacity_in_ms, int sample_rate_hz, int
num_channel) {
  audio_format_.SetSampleRate(sample_rate_hz).SetChannels(num_channel);
  RTC_DCHECK(audio_format_.ValidPcmCheck()) << audio_format_.ToString();
  ExpandCapacityIfNeed(capacity_in_ms);
}

void AudioFrameLite::Reset(size_t capacity_in_ms, const AudioFormatInfo&
audio_format) { audio_format_ = audio_format;
  RTC_DCHECK(audio_format_.ValidPcmCheck()) << audio_format_.ToString();
  ExpandCapacityIfNeed(capacity_in_ms);
}

void AudioFrameLite::ExpandCapacityIfNeed(size_t capacity_in_ms) {
  size_t capacity_in_byte = audio_format_.AudioMsToByteSize(capacity_in_ms);
  if (capacity_in_byte > capacity_in_byte_) {
    delete[] data_;
    capacity_in_byte_ = capacity_in_byte;
    data_ = new uint8_t[capacity_in_byte_];
  }
}*/

}  // namespace core
