#include "core/audio/audio_source_to_mix_source.h"

#include <functional>

#include "webrtc/api/audio/channel_layout.h"

#include "core/audio/audio_frame_util.h"
namespace yuanli {
static std::atomic_int sg_ssrc(0);
AudioSourceToMixSource::AudioSourceToMixSource(
    core::AudioRawSource* audio_raw_source, std::function<void()> on_remove)
    : ssrc_(sg_ssrc++),
      audio_raw_source_(audio_raw_source),
      on_remove_(on_remove),
      audio_frame_lite_(10_ms, audio_raw_source
                                   ? audio_raw_source->GetAudioFormatInfo()
                                   : core::kNullAudioFormatInfo) {
  RTC_DCHECK(audio_frame_lite_.AudioFormat().ValidPcmCheck());
  RTC_DCHECK(audio_raw_source_) << "audio_raw_source_ is nullptr";
}

core::AudioMixer::Source::AudioFrameInfo
AudioSourceToMixSource::GetAudioFrameWithInfo(int sample_rate_hz,
                                              core::AudioFrame* audio_frame) {
  if (!audio_raw_source_) {
    return core::AudioMixer::Source::AudioFrameInfo::kError;
  }
  audio_frame_lite_.ResetReadableSize(10_ms);
  auto read_size = audio_raw_source_->Read(audio_frame_lite_.MutableData(),
                                           audio_frame_lite_.CapacityInByte());
  if (read_size < audio_frame_lite_.CapacityInByte()) {
    if (audio_raw_source_->GetSourceStatus() == core::SourceStatus::kStatic) {
      if (on_remove_) {
        on_remove_();
      }
    }
  }
  if (read_size == 0) {
    return core::AudioMixer::Source::AudioFrameInfo::kMuted;
  }
  if (sample_rate_hz !=
      audio_raw_source_->GetAudioFormatInfo().GetSampleRateToInt()) {
    if (!audio_converter_) {
      audio_converter_ = std::make_unique<core::AudioConverterFFmpeg>();
    }
    audio_frame->SetSampleRateAndChannelSize(sample_rate_hz);
    auto num_channels = audio_frame_lite_.AudioFormat().GetChannelsCount();
    audio_frame->SetLayoutAndNumChannels(core::GuessChannelLayout(num_channels),
                                         num_channels);
    core::AudioFrameLiteDelegate delegate =
        core::audio_util::CreateAudioFrameLiteDelegate(audio_frame);
    audio_converter_->Convert(audio_frame_lite_, delegate);
  } else {
    core::audio_util::AudioFrameLiteToAudioFrame(audio_frame_lite_,
                                                 *audio_frame);
  }
  return core::AudioMixer::Source::AudioFrameInfo::kNormal;
}

int AudioSourceToMixSource::PreferredSampleRate() const {
  if (audio_raw_source_) {
    return audio_raw_source_->GetAudioFormatInfo().GetSampleRateToInt();
  }
  return 0;
}

}  // namespace yuanli
