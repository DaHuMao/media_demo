#ifndef WEBRTC_AUDIO_AUDIO_FORMAT_DEFINE_H_
#define WEBRTC_AUDIO_AUDIO_FORMAT_DEFINE_H_

#include <stdint.h>

#include "core/audio/audio_common_types_define.h"
#include "util/macro_defines.h"
#include "util/time_to_class.h"

namespace core {

class COMMON_DLLEXPORT AudioFormatInfo {
 public:
  constexpr AudioFormatInfo() = default;
  constexpr AudioFormatInfo(AudioSampleRate sample_rate,
                            AudioChannelLayout channels,
                            AudioSampleFormat sample_format =
                                AudioSampleFormat::kAudioSampleFormatPcmInt16)
      : sample_rate_hz_(sample_rate),
        channel_layout_(channels),
        audio_sample_format_(sample_format) {}
  AudioFormatInfo(int sample_rate, int channels,
                  AudioSampleFormat sample_format =
                      AudioSampleFormat::kAudioSampleFormatPcmInt16);

  AudioFormatInfo &SetSampleRate(AudioSampleRate sample_rate);
  AudioFormatInfo &SetChannelLayout(AudioChannelLayout channels_layout);
  AudioFormatInfo &SetSampleFormat(AudioSampleFormat sample_format);
  AudioFormatInfo &SetSampleRate(int sample_rate);
  AudioFormatInfo &SetChannels(int channels);
  bool operator==(const AudioFormatInfo &info) const;
  bool operator!=(const AudioFormatInfo &info) const;
  void ResetFormat(const AudioFormatInfo &info);
  int GetSampleRateToInt() const;
  int GetChannelsCount() const;
  AudioSampleRate GetSampleRate() const;
  AudioChannelLayout GetChannelLayout() const;
  AudioSampleFormat GetAudioSampleFormat() const;
  size_t ByteSizePerSample() const;
  size_t ByteSizePerFrame() const;
  bool MaybeSameAudioFormat(int sample_rate_hz, int channels) const;
  bool IsPlanner() const;
  std::string ToString() const;

  //--------- SizeToMs MsToSize -------//

  size_t AudioMsToFrameSize(util::MillisecondsClass time_ms) const;
  size_t AudioMsToSampleSize(util::MillisecondsClass time_ms) const;
  size_t AudioMsToByteSize(util::MillisecondsClass time_ms) const;
  util::MillisecondsClass AudioFrameSizeToMs(size_t audio_size_in_frame) const;
  util::MillisecondsClass AudioSampleSizeToMs(
      size_t audio_size_in_sample) const;
  util::MillisecondsClass AudioByteSizeToMs(size_t audio_size_in_byte) const;
  size_t AudioConvertByteSize(const AudioFormatInfo &src_format,
                              size_t src_size) const;
  //--------- SizeToMs MsToSize -------//

  //------------ Check --------------//
  bool ValidPcmInt16Check() const;
  bool ValidPcmCheck() const;
  bool ValidSampleRateAndChannelCheck() const;
  //------------ Check --------------//

 private:
  AudioSampleRate sample_rate_hz_ = AudioSampleRate::kNull;
  AudioChannelLayout channel_layout_ = AudioChannelLayout::kNull;
  AudioSampleFormat audio_sample_format_ =
      AudioSampleFormat::kAudioSampleFormatNone;
};
constexpr AudioFormatInfo kNullAudioFormatInfo;
constexpr AudioFormatInfo k48kStereoPcm16 = AudioFormatInfo(
    AudioSampleRate::kSampleRate48k, AudioChannelLayout::kStereo);
constexpr AudioFormatInfo k48kMonoPcm16 =
    AudioFormatInfo(AudioSampleRate::kSampleRate48k, AudioChannelLayout::kMono);
constexpr AudioFormatInfo k16kStereoPcm16 = AudioFormatInfo(
    AudioSampleRate::kSampleRate16k, AudioChannelLayout::kStereo);
constexpr AudioFormatInfo k16kMonoPcm16 =
    AudioFormatInfo(AudioSampleRate::kSampleRate16k, AudioChannelLayout::kMono);
std::ostream &operator<<(std::ostream &os, const core::AudioFormatInfo &info);
}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_FORMAT_DEFINE_H_
