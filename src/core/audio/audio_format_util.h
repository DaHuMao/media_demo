#ifndef WEBRTC_AUDIO_AUDIO_FORMAT_UTIL_H_
#define WEBRTC_AUDIO_AUDIO_FORMAT_UTIL_H_
#include "webrtc/api/array_view.h"

#include "core/audio/audio_common_types_define.h"
namespace core {
namespace audio_util {
AudioSampleRate IntToAudioSampleRate(int sample_rate_hz);
AudioChannelLayout IntToAudioChannelLayout(int num_channels);
int AudioChannlsLayoutToInt(AudioChannelLayout channel_layout);
int AudioSampleRateToInt(AudioSampleRate audio_sample_rate);
const char *AudioSampleFormatName(AudioSampleFormat fmt);
bool IsPlanar(AudioSampleFormat fmt);
AudioSampleFormat PlanarToInterleaved(AudioSampleFormat fmt);
size_t ByteSizePerSample(AudioSampleFormat fmt);
rtc::ArrayView<const AudioSampleFormat> AudioSampleFormatArray();
bool ValidSampleRate(int sample_rate_hz);
bool ValidChannel(int channels);
}  // namespace audio_util
}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_FORMAT_UTIL_H_
