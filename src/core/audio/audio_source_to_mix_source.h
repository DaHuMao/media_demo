#ifndef BIZ_AUDIO_SRC_AUDIO_SOURCE_TO_MIX_SOURCE_H_
#define BIZ_AUDIO_SRC_AUDIO_SOURCE_TO_MIX_SOURCE_H_
#include <functional>
#include <memory>

#include "webrtc/api/audio/audio_mixer.h"

#include "core/audio/audio_converter.h"
#include "core/audio/audio_frame.h"
#include "core/audio/audio_io_define.h"
namespace core {
class AudioSourceToMixSource : public core::AudioMixer::Source {
 public:
  AudioSourceToMixSource(core::AudioRawSource* audio_raw_source,
                         std::function<void()> on_remove = nullptr);
  AudioSourceToMixSource(const AudioSourceToMixSource&&);
  AudioSourceToMixSource& operator=(const AudioSourceToMixSource&&);
  ~AudioSourceToMixSource() = default;
  AudioFrameInfo GetAudioFrameWithInfo(int sample_rate_hz,
                                       core::AudioFrame* audio_frame) override;
  int Ssrc() const override { return ssrc_; }
  int PreferredSampleRate() const override;

 private:
  int ssrc_ = 0;
  core::AudioRawSource* audio_raw_source_;
  std::function<void()> on_remove_;
  core::AudioFrameLite audio_frame_lite_;
  std::unique_ptr<core::AudioConverterFFmpeg> audio_converter_;
};
}  // namespace core
#endif  // BIZ_AUDIO_SRC_AUDIO_SOURCE_TO_MIX_SOURCE_H_
