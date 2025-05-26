#include "core/audio/audio_mixer_impl.h"

#include "core/audio/audio_format_define.h"
#include "core/audio/audio_frame_util.h"
namespace core {
AudioMixerImpl::AudioMixerImpl(std::function<void()> on_all_remove)
    : limiter_(core::k48kMonoPcm16.AudioMsToFrameSize(10_ms), "AudioMixer"),
      on_all_remove_(on_all_remove) {}

size_t AudioMixerImpl::Read(void* data, size_t read_size,
                            const AudioFormatInfo& info) {
  if (!info.ValidPcmInt16Check() || read_size == 0) {
    return 0;
  }
  return MixFrame(data, read_size, info);
}

int32_t AudioMixerImpl::AddSource(core::AudioRawSource* source,
                                  std::function<void()> on_remove) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (source_map_.find(source) != source_map_.end()) {
    return -1;
  }
  source_map_.insert({source, SourceWrapper(source, on_remove)});
  return 0;
}

int32_t AudioMixerImpl::RemoveSource(core::AudioRawSource* source) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = source_map_.find(source);
  if (iter == source_map_.end()) {
    return -1;
  }
  source_map_.erase(iter);
  return 0;
}

size_t AudioMixerImpl::SourceCount() {
  std::lock_guard<std::mutex> lock(mutex_);
  return source_map_.size();
}

size_t AudioMixerImpl::MixFrame(void* data, size_t should_read_size,
                                const AudioFormatInfo& info) {
  auto read_size = 0;
  AudioFrameLiteDelegate audio_frame(reinterpret_cast<uint8_t*>(data),
                                     should_read_size, should_read_size, info);
  std::vector<core::AudioRawSource*> remove_list;
  std::vector<std::function<void()>> on_remove_list;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (source_map_.size() == 1) {
      auto& source_wrapper = source_map_.begin()->second;
      read_size = ReadFrame(source_wrapper, audio_frame, remove_list);
    } else if (source_map_.size() > 1) {
      if (info != last_audio_format_info_) {
        last_audio_format_info_ = info;
      }
      if (mix_buffer_.capacity() < audio_frame.SampleSize()) {
        mix_buffer_.resize(audio_frame.SampleSize());
      }
      if (read_buffer_.capacity() < audio_frame.ByteSize()) {
        read_buffer_.resize(audio_frame.ByteSize());
      }
      AudioFrameLiteDelegate delegate(read_buffer_.data(),
                                      audio_frame.ByteSize(),
                                      read_buffer_.capacity(), info);
      memset(mix_buffer_.data(), 0, mix_buffer_.capacity() * sizeof(float));
      for (auto& source : source_map_) {
        auto& source_wrapper = source.second;
        ReadFrame(source_wrapper, delegate, remove_list);
        audio_util::AudioFrameToFloatS16(delegate, mix_buffer_.data(),
                                         audio_frame.SampleSize(),
                                         /*float_data_is_planar=*/true,
                                         /*is_add_to=*/true);
      }
      webrtc::DeinterleavedView<float> audio_frame_view(
          mix_buffer_.data(), audio_frame.FrameSize(),
          info.GetChannelsCount());
      limiter_.SetSamplesPerChannel(audio_frame.FrameSize());
      limiter_.Process(audio_frame_view);
      audio_util::FloatS16ToAudioFrame(
          mix_buffer_.data(), audio_frame.SampleSize(),
          /*float_data_is_planar=*/true, audio_frame);
      read_size = audio_frame.ByteSize();
    }
    for (auto& source : remove_list) {
      auto it = source_map_.find(source);
      RTC_DCHECK(it != source_map_.end());
      on_remove_list.push_back(it->second.on_remove);
      source_map_.erase(it);
    }
    if (source_map_.empty() && on_all_remove_) {
      on_all_remove_();
    }
  }
  for (auto& remove : on_remove_list) {
    if (remove) {
      remove();
    }
  }
  return read_size;
}
size_t AudioMixerImpl::ReadFrame(SourceWrapper& source_wrapper,
                                 core::AudioFrameLiteDelegate& audio_frame,
                                 std::vector<AudioRawSource*>& remove_list) {
  auto should_read_size = audio_frame.CapacityInByte();
  auto read_size = source_wrapper.audio_resample_audio_source->Read(
      audio_frame.MutableData(), should_read_size, audio_frame.AudioFormat());
  if (read_size < should_read_size &&
      source_wrapper.audio_resample_audio_source->GetSourceStatus() ==
          core::SourceStatus::kStatic) {
    source_wrapper.start_gain = 1.0f;
    source_wrapper.end_gain = 0.0f;
    remove_list.push_back(source_map_.begin()->first);
  }
  if (source_wrapper.start_gain != source_wrapper.end_gain) {
    audio_util::Ramp(source_wrapper.start_gain, source_wrapper.end_gain,
                     audio_frame);
    source_wrapper.start_gain = source_wrapper.end_gain;
  }
  return read_size;
}

std::unique_ptr<AudioMixer> AudioMixer::Create(
    std::function<void()> on_all_remove) {
  return std::make_unique<AudioMixerImpl>(on_all_remove);
}

}  // namespace core
