#ifndef CORE_AUDIO_WEBRTC_AUDIO_FRAME_UTIL_H_
#define CORE_AUDIO_WEBRTC_AUDIO_FRAME_UTIL_H_
#include "webrtc/api/audio/audio_frame.h"
#include "core/audio/audio_frame.h"
namespace core {
namespace audio_util {
AudioFrameLiteDelegate CreateAudioFrameLiteDelegate(
    webrtc::AudioFrame *audio_frame);
std::unique_ptr<AudioFrameLiteView> CreateAudioFrameLiteView(
    const webrtc::AudioFrame *audio_frame);
void FormatAudioFrame(const AudioFormatInfo &audio_format,
                      webrtc::AudioFrame &audio_frame);
void AudioFrameLiteToAudioFrame(const AudioFrameLiteView &audio_frame_in,
                                webrtc::AudioFrame &audio_frame);
void AudioFrameToAudioFrameLite(const webrtc::AudioFrame &audio_frame_in,
                                AudioFrameLiteDelegate &audio_frame);
void ScaleAudio(webrtc::AudioFrame *audio_frame, float coefficient);
} // namespace audio_util
}  // namespace core
#endif  // CORE_AUDIO_WEBRTC_AUDIO_FRAME_UTIL_H_
