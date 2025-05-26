#include "core/adapter/audio_frame_ffmpeg.h"

#include "core/adapter/audio_format_ffmpeg_convert.h"
#include "core/audio/audio_format_util.h"
namespace core {
AudioFrameMaybePlanarView AudioFrameFfmpeg::GetAudioFrameLiteView(
    const core::AvFrameWrapper* frame_wrapper) {
  auto frame = frame_wrapper->GetFrame();
  auto audio_format = core::AudioFormatInfo(
      core::audio_util::IntToAudioSampleRate(frame->sample_rate),
      ffmpeg_util::GetChannelLayout(frame->channel_layout),
      ffmpeg_util::GetSampleFormat(static_cast<AVSampleFormat>(frame->format)));
  auto one_dim_size = frame->nb_samples * audio_format.ByteSizePerSample();
  std::vector<const uint8_t*> data_arr;
  data_arr.push_back(frame->data[0]);
  if (audio_format.IsPlanner()) {
    for (int i = 1; i < frame->channels; i++) {
      data_arr.push_back(frame->data[i]);
    }
  }
  return AudioFrameMaybePlanarView(data_arr, one_dim_size, audio_format);
}
}  // namespace core
