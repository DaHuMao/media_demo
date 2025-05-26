
#include "core/audio/audio_format_define.h"

#include <sstream>

#include "webrtc/rtc_base/checks.h"

#include "core/audio/audio_format_util.h"
#include "util/log.h"

namespace core {
static constexpr char const *kTag = "AudioFormatInfo";

AudioFormatInfo::AudioFormatInfo(int sample_rate, int channels,
                                 AudioSampleFormat sample_format)
    : sample_rate_hz_(audio_util::IntToAudioSampleRate(sample_rate)),
      channel_layout_(audio_util::IntToAudioChannelLayout(channels)),
      audio_sample_format_(sample_format) {}

bool AudioFormatInfo::operator==(const AudioFormatInfo &info) const {
  return this->sample_rate_hz_ == info.GetSampleRate() &&
         this->channel_layout_ == info.GetChannelLayout() &&
         this->audio_sample_format_ == info.GetAudioSampleFormat();
}

AudioFormatInfo &AudioFormatInfo::SetSampleRate(AudioSampleRate sample_rate) {
  sample_rate_hz_ = sample_rate;
  RTC_DCHECK(AudioSampleRate::kNull != sample_rate_hz_)
      << "invalid sample_rate";
  return *this;
}

AudioFormatInfo &AudioFormatInfo::SetChannelLayout(
    AudioChannelLayout channels_layout) {
  channel_layout_ = channels_layout;
  RTC_DCHECK(AudioChannelLayout::kNull != channel_layout_)
      << "invalid channels_layout";
  return *this;
}

AudioFormatInfo &AudioFormatInfo::SetSampleFormat(
    AudioSampleFormat sample_format) {
  audio_sample_format_ = sample_format;
  RTC_DCHECK(AudioSampleFormat::kAudioSampleFormatNone != audio_sample_format_)
      << "invalid format";
  return *this;
}

AudioFormatInfo &AudioFormatInfo::SetSampleRate(int sample_rate) {
  sample_rate_hz_ = audio_util::IntToAudioSampleRate(sample_rate);
  RTC_DCHECK(audio_util::ValidSampleRate(sample_rate))
      << "invalid sample_rate: " << sample_rate;
  return *this;
}

AudioFormatInfo &AudioFormatInfo::SetChannels(int channels) {
  channel_layout_ = audio_util::IntToAudioChannelLayout(channels);
  RTC_DCHECK(audio_util::ValidChannel(channels))
      << "invalid channels: " << channels;
  return *this;
}

bool AudioFormatInfo::operator!=(const AudioFormatInfo &info) const {
  return !this->operator==(info);
}

void AudioFormatInfo::ResetFormat(const AudioFormatInfo &info) {
  sample_rate_hz_ = info.GetSampleRate();
  channel_layout_ = info.GetChannelLayout();
  audio_sample_format_ = info.GetAudioSampleFormat();
}

int AudioFormatInfo::GetSampleRateToInt() const {
  return audio_util::AudioSampleRateToInt(sample_rate_hz_);
}

int AudioFormatInfo::GetChannelsCount() const {
  return audio_util::AudioChannlsLayoutToInt(channel_layout_);
}

AudioSampleRate AudioFormatInfo::GetSampleRate() const {
  return sample_rate_hz_;
}

AudioChannelLayout AudioFormatInfo::GetChannelLayout() const {
  return channel_layout_;
}

AudioSampleFormat AudioFormatInfo::GetAudioSampleFormat() const {
  return audio_sample_format_;
}

size_t AudioFormatInfo::ByteSizePerSample() const {
  return audio_util::ByteSizePerSample(audio_sample_format_);
}
size_t AudioFormatInfo::ByteSizePerFrame() const {
  return GetChannelsCount() * ByteSizePerSample();
}

bool AudioFormatInfo::MaybeSameAudioFormat(int sample_rate_hz,
                                           int channels) const {
  return audio_util::IntToAudioSampleRate(sample_rate_hz) == sample_rate_hz_ &&
         audio_util::IntToAudioChannelLayout(channels) == channel_layout_;
}
bool AudioFormatInfo::IsPlanner() const {
  return audio_util::IsPlanar(audio_sample_format_);
}

std::string AudioFormatInfo::ToString() const {
  const char *sample_format_name =
      audio_util::AudioSampleFormatName(audio_sample_format_);
  std::ostringstream oss;
  oss << "sample_rate_hz_: " << GetSampleRateToInt()
      << " channels_count: " << GetChannelsCount()
      << " sample_format: " << sample_format_name;
  return oss.str();
}

//--------- SizeToMs MsToSize -------//
size_t AudioFormatInfo::AudioMsToFrameSize(
    util::MillisecondsClass time_ms) const {
  return static_cast<uint64_t>(GetSampleRateToInt()) * time_ms.Value() / 1000;
}

size_t AudioFormatInfo::AudioMsToSampleSize(
    util::MillisecondsClass time_ms) const {
  return AudioMsToFrameSize(time_ms) * GetChannelsCount();
}

size_t AudioFormatInfo::AudioMsToByteSize(
    util::MillisecondsClass time_ms) const {
  return AudioMsToSampleSize(time_ms) * ByteSizePerSample();
}

util::MillisecondsClass AudioFormatInfo::AudioFrameSizeToMs(
    size_t audio_size_in_frame) const {
  return util::MillisecondsClass(
      static_cast<int64_t>(audio_size_in_frame * 1000) / GetSampleRateToInt());
}

util::MillisecondsClass AudioFormatInfo::AudioSampleSizeToMs(
    size_t audio_size_in_sample) const {
  return AudioFrameSizeToMs(audio_size_in_sample / GetChannelsCount());
}

util::MillisecondsClass AudioFormatInfo::AudioByteSizeToMs(
    size_t audio_size_in_byte) const {
  return AudioFrameSizeToMs(audio_size_in_byte / ByteSizePerFrame());
}

size_t AudioFormatInfo::AudioConvertByteSize(const AudioFormatInfo &src_format,
                                             size_t src_size) const {
  return src_size * AudioMsToByteSize(100_ms) /
         src_format.AudioMsToByteSize(100_ms);
}
//--------- SizeToMs MsToSize -------//

//------------ Check --------------//
bool AudioFormatInfo::ValidPcmInt16Check() const {
  if (AudioSampleRate::kNull != sample_rate_hz_ &&
      AudioChannelLayout::kNull != channel_layout_ &&
      AudioSampleFormat::kAudioSampleFormatPcmInt16 == audio_sample_format_) {
    return true;
  }
  LOGE_TAG(kTag) << "failed ValidPcmInt16Check: " << ToString();
  return false;
}

bool AudioFormatInfo::ValidPcmCheck() const {
  if (AudioSampleRate::kNull != sample_rate_hz_ &&
      AudioChannelLayout::kNull != channel_layout_ &&
      AudioSampleFormat::kAudioSampleFormatNone != audio_sample_format_) {
    return true;
  }
  LOGE_TAG(kTag) << "faild ValidPcmCheck: " << ToString();
  return false;
}

bool AudioFormatInfo::ValidSampleRateAndChannelCheck() const {
  if (AudioSampleRate::kNull != sample_rate_hz_ &&
      AudioChannelLayout::kNull != channel_layout_) {
    return true;
  }
  LOGE_TAG(kTag) << "faild ValidSampleRateAndChannelCheck: " << ToString();
  return false;
}
//------------ Check --------------//

std::ostream &operator<<(std::ostream &os, const core::AudioFormatInfo &info) {
  os << info.ToString();
  return os;
}
}  // namespace core
