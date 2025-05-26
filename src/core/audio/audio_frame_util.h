#ifndef WEBRTC_AUDIO_AUDIO_FRAME_UTIL_H_
#define WEBRTC_AUDIO_AUDIO_FRAME_UTIL_H_
#include "core/audio/audio_frame.h"
namespace core {
namespace audio_util {
float GetAudioFrameEnerge(const AudioFrameLiteView &audio_frame);
float GetAudioFrameEnergeDb(const AudioFrameLiteView &audio_frame);
void Ramp(float start_gain, float end_gain,
          AudioFrameLiteDelegate &audio_frame);
void FloatS16ToAudioFrame(const float *data, size_t data_size,
                          bool float_data_is_planar,
                          AudioFrameLiteDelegate &audio_frame);
void AudioFrameToFloatS16(const AudioFrameLiteView &audio_frame, float *data,
                          size_t data_size, bool float_data_is_planar,
                          bool is_add_to = false);
}  // namespace audio_util
}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_FRAME_UTIL_H_
