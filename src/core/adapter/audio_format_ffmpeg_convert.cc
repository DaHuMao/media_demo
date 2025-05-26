#include "core/adapter/audio_format_ffmpeg_convert.h"

#include "util/array_find.h"
extern "C" {
#include "libavutil/channel_layout.h"
}
namespace ffmpeg_util {
static constexpr std::pair<core::AudioSampleFormat, AVSampleFormat>
    ffmpeg_sample_fmt_map[] = {
        {core::AudioSampleFormat::kAudioSampleFormatPcmInt16,
         AV_SAMPLE_FMT_S16},
        {core::AudioSampleFormat::kAudioSampleFormatPcmFloat,
         AV_SAMPLE_FMT_FLT},
        {core::AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar,
         AV_SAMPLE_FMT_FLTP},
        {core::AudioSampleFormat::kAudioSampleFormatPcmInt16Planar,
         AV_SAMPLE_FMT_S16P}};

static constexpr std::pair<core::AudioChannelLayout, int32_t>
    ffmpeg_channel_layout_map[] = {
        {core::AudioChannelLayout::kMono, AV_CH_LAYOUT_MONO},
        {core::AudioChannelLayout::kStereo, AV_CH_LAYOUT_STEREO}};

AVSampleFormat GetFFmpegSampleFormat(core::AudioSampleFormat sample_format) {
  return util::ArrayFind(ffmpeg_sample_fmt_map, sample_format,
                         AV_SAMPLE_FMT_NONE);
}

core::AudioSampleFormat GetSampleFormat(AVSampleFormat sample_format) {
  return util::ArrayFindKey(ffmpeg_sample_fmt_map, sample_format,
                            core::AudioSampleFormat::kAudioSampleFormatNone);
}

int32_t GetFFmpegChannelLayout(core::AudioChannelLayout channel_layout) {
  return util::ArrayFind(ffmpeg_channel_layout_map, channel_layout,
                         AV_CH_LAYOUT_MONO);
}

core::AudioChannelLayout GetChannelLayout(int32_t channel_layout) {
  return util::ArrayFindKey(ffmpeg_channel_layout_map, channel_layout,
                            core::AudioChannelLayout::kNull);
}
}  // namespace ffmpeg_util
