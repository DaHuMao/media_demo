#ifndef CORE_AUDIO_AUDIO_MIXER_IMPL_H_
#define CORE_AUDIO_AUDIO_MIXER_IMPL_H_
#include <mutex>
#include <unordered_map>
#include <vector>

#include "core/audio/audio_mixer.h"
#include "core/audio/resample_io.h"
#include "modules/audio_processing/agc2/limiter.h"
namespace core {
class AudioMixerImpl final : public AudioMixer {
 public:
  AudioMixerImpl(std::function<void()> on_all_remove);
  ~AudioMixerImpl() override = default;
  size_t Read(void* data,
      size_t read_size, const AudioFormatInfo& info) override;
  int32_t AddSource(core::AudioRawSource* source,
                    std::function<void()> on_remove) override;
  int32_t RemoveSource(core::AudioRawSource* source) override;
  size_t SourceCount() override;

 private:
  struct SourceWrapper {
    SourceWrapper(core::AudioRawSource* source, std::function<void()> on_remove)
        : audio_resample_audio_source(
              std::make_unique<core::AudioResampleAudioSource>(source)),
          on_remove(on_remove) {}
    std::unique_ptr<core::AudioResampleAudioSource> audio_resample_audio_source;
    std::function<void()> on_remove;
    float start_gain = 0.0f;
    float end_gain = 1.0f;
  };
  size_t MixFrame(void* data,
      size_t read_size, const AudioFormatInfo& info);
  size_t ReadFrame(SourceWrapper& source_wrapper,
                   core::AudioFrameLiteDelegate& audio_frame,
                   std::vector<AudioRawSource*>& remove_list);
  std::mutex mutex_;
  core::AudioFormatInfo last_audio_format_info_;
  std::unordered_map<core::AudioRawSource*, SourceWrapper> source_map_;
  webrtc::Limiter limiter_;
  std::vector<float> mix_buffer_;
  std::vector<uint8_t> read_buffer_;
  std::function<void()> on_all_remove_;
};
}  // namespace core
#endif  // CORE_AUDIO_AUDIO_MIXER_IMPL_H_
