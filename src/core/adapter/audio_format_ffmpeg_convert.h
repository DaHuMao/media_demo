#ifndef CORE_ADAPTER_AUDIO_FORMAT_FFMPEG_CONVERT_H_
#define CORE_ADAPTER_AUDIO_FORMAT_FFMPEG_CONVERT_H_
#include "core/audio/audio_common_types_define.h"
extern "C" {
#include "libavutil/samplefmt.h"
}
namespace ffmpeg_util {
AVSampleFormat GetFFmpegSampleFormat(core::AudioSampleFormat sample_format);
core::AudioSampleFormat GetSampleFormat(AVSampleFormat sample_format);
int32_t GetFFmpegChannelLayout(core::AudioChannelLayout channel_layout);
core::AudioChannelLayout GetChannelLayout(int32_t channel_layout);
}  // namespace ffmpeg_util
#endif // CORE_ADAPTER_AUDIO_FORMAT_FFMPEG_CONVERT_H_
