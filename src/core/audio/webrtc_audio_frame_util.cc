#include "core/audio/webrtc_audio_frame_util.h"
#include "webrtc/rtc_base/checks.h"
namespace core {
namespace audio_util {
constexpr util::MillisecondsClass kAudioFrameFixedFrameSizeInMs = 10_ms;
AudioFrameLiteDelegate CreateAudioFrameLiteDelegate(
    webrtc::AudioFrame *audio_frame) {
  return AudioFrameLiteDelegate(
      reinterpret_cast<uint8_t *>(audio_frame->mutable_data()),
      audio_frame->samples_per_channel_ * audio_frame->num_channels_ *
          sizeof(int16_t),
      webrtc::AudioFrame::kMaxDataSizeBytes,
      AudioFormatInfo(audio_frame->sample_rate_hz_,
                      static_cast<int>(audio_frame->num_channels_)));
}

class AudioFrameLiteViewDriver : public AudioFrameLiteView {
 public:
  AudioFrameLiteViewDriver(const webrtc::AudioFrame *audio_frame)
      : audio_format_(
            AudioFormatInfo(audio_frame->sample_rate_hz_,
                            static_cast<int>(audio_frame->num_channels_))) {
    RTC_DCHECK(audio_format_.ValidPcmCheck()) << audio_format_.ToString();
    audio_format_ptr_ = &audio_format_;
    const_data_ = audio_frame->data();
    byte_size_ =
        audio_frame->samples_per_channel_ * audio_format_.ByteSizePerFrame();
  }

 private:
  AudioFormatInfo audio_format_;
};

std::unique_ptr<AudioFrameLiteView> CreateAudioFrameLiteView(
    const webrtc::AudioFrame *audio_frame) {
  return std::make_unique<AudioFrameLiteViewDriver>(audio_frame);
}

void FormatAudioFrame(const AudioFormatInfo &audio_format,
                      webrtc::AudioFrame &audio_frame) {
  audio_frame.sample_rate_hz_ = audio_format.GetSampleRateToInt();
  audio_frame.num_channels_ = audio_format.GetChannelsCount();
  audio_frame.samples_per_channel_ =
      audio_format.AudioMsToFrameSize(kAudioFrameFixedFrameSizeInMs);
}

void AudioFrameLiteToAudioFrame(const AudioFrameLiteView &audio_frame_in,
                                webrtc::AudioFrame &audio_frame) {
  RTC_DCHECK(audio_frame_in.SizeInMs() == kAudioFrameFixedFrameSizeInMs);
  audio_frame.UpdateFrame(
      audio_frame.timestamp_,
      reinterpret_cast<const int16_t *>(audio_frame_in.Data()),
      audio_frame_in.FrameSize(),
      audio_frame_in.AudioFormat().GetSampleRateToInt(),
      audio_frame.speech_type_, audio_frame.vad_activity_,
      audio_frame_in.AudioFormat().GetChannelsCount());
}

void AudioFrameToAudioFrameLite(const webrtc::AudioFrame &audio_frame_in,
                                AudioFrameLiteDelegate &audio_frame) {
  audio_frame.MutableAudioFormat() =
      AudioFormatInfo(audio_frame_in.sample_rate_hz_,
                      static_cast<int>(audio_frame_in.num_channels_));
  audio_frame.CopyFrom(audio_frame_in.data(),
                       audio_frame_in.samples_per_channel_ * sizeof(int16_t) *
                           audio_frame_in.num_channels_);
}

void ScaleAudio(webrtc::AudioFrame *audio_frame, float coefficient) {
  if (std::abs(1.0 - coefficient) < 0.01) {
    return;
  }
  int16_t *ptr = audio_frame->mutable_data();
  for (size_t i = 0;
       i < audio_frame->samples_per_channel_ * audio_frame->num_channels_;
       ++i) {
    ptr[i] *= coefficient;
  }
}
}  // namespace audio_util
}  // namespace core
