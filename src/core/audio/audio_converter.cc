#include "core/audio/audio_converter.h"

#include <cstring>

#include "core/audio/audio_format_define.h"
#include "rtc_base/checks.h"
#include "util/log.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}
namespace core {
namespace {
const std::string kLogTag = "AudioConverterFFmpeg";
const int kAudioInputChannelMaxNums = 8;
const int kAudioOutputChannelMaxNums = 2;

static std::unordered_map<int, int> ffmpeg_channel_layout_map = {
    {1, AV_CH_LAYOUT_MONO},    {2, AV_CH_LAYOUT_STEREO},
    {3, AV_CH_LAYOUT_2POINT1}, {4, AV_CH_LAYOUT_4POINT0},
    {5, AV_CH_LAYOUT_5POINT0}, {6, AV_CH_LAYOUT_6POINT0},
    {7, AV_CH_LAYOUT_7POINT0}, {8, AV_CH_LAYOUT_7POINT1}};

int GetFFmpegChannelLayout(int number_of_channels) {
  if (ffmpeg_channel_layout_map.find(number_of_channels) !=
      ffmpeg_channel_layout_map.end()) {
    return ffmpeg_channel_layout_map[number_of_channels];
  }
  return -1;
}

static std::unordered_map<core::AudioSampleFormat, AVSampleFormat>
    ffmpeg_sample_fmt_map = {
        {core::AudioSampleFormat::kAudioSampleFormatPcmInt16,
         AV_SAMPLE_FMT_S16},
        {core::AudioSampleFormat::kAudioSampleFormatPcmFloat,
         AV_SAMPLE_FMT_FLT},
        {core::AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar,
         AV_SAMPLE_FMT_FLTP},
        {core::AudioSampleFormat::kAudioSampleFormatPcmInt16Planar,
         AV_SAMPLE_FMT_S16P}};

AVSampleFormat GetFFmpegSampleFormat(core::AudioSampleFormat sample_format) {
  if (ffmpeg_sample_fmt_map.find(sample_format) !=
      ffmpeg_sample_fmt_map.end()) {
    return ffmpeg_sample_fmt_map[sample_format];
  }
  return AV_SAMPLE_FMT_NONE;
}

}  // namespace

AudioConverterFFmpeg::AudioConverterFFmpeg(bool first_frame_fill_zero)
    : swr_ctx_(nullptr),
      inited_(false),
      first_frame_fill_zero_(first_frame_fill_zero) {}

AudioConverterFFmpeg::~AudioConverterFFmpeg() { UnInit(); }

bool AudioConverterFFmpeg::ReinitIfNeeded(const AudioFormatInfo &in,
                                          const AudioFormatInfo &out) {
  if (in == format_in_ && out == format_out_ && inited_) {
    return true;
  }

  UnInit();
  if (!Init(in, out)) {
    RTC_DCHECK(false) << "init faild "
                      << "inited_" << inited_ << "format_in_: " << in.ToString()
                      << " format_out_: " << out.ToString();
    return false;
  }
  format_in_ = in;
  format_out_ = out;
  return true;
}

bool AudioConverterFFmpeg::Init(const AudioFormatInfo &in,
                                const AudioFormatInfo &out) {
  RTC_DCHECK(in.GetChannelsCount() <= kAudioInputChannelMaxNums);
  RTC_DCHECK(out.GetChannelsCount() <= kAudioOutputChannelMaxNums);

  if (inited_) {
    return false;
  }

  if (in.GetChannelsCount() > kAudioInputChannelMaxNums) {
    LOGE_TAG(kLogTag)
        << "failed to init audio converter, number of input channel:"
        << in.GetChannelsCount()
        << " is over kAudioInputChannelMaxNums:" << kAudioInputChannelMaxNums;
    return false;
  }

  if (out.GetChannelsCount() > kAudioOutputChannelMaxNums) {
    LOGE_TAG(kLogTag)
        << "failed to init audio converter, number of output channel:"
        << out.GetChannelsCount()
        << " is over kAudioOutputChannelMaxNums:" << kAudioOutputChannelMaxNums;
    return false;
  }

  int ffmpeg_input_channel_layout =
      GetFFmpegChannelLayout(in.GetChannelsCount());
  int ffmpeg_output_channel_layout =
      GetFFmpegChannelLayout(out.GetChannelsCount());
  int ffmpeg_input_sample_rate_hz = in.GetSampleRateToInt();
  int ffmpeg_output_sample_rate_hz = out.GetSampleRateToInt();
  AVSampleFormat ffmpeg_input_sample_format =
      GetFFmpegSampleFormat(in.GetAudioSampleFormat());
  AVSampleFormat ffmpeg_output_sample_format =
      GetFFmpegSampleFormat(out.GetAudioSampleFormat());

  swr_ctx_ = swr_alloc_set_opts(
      nullptr, ffmpeg_output_channel_layout, ffmpeg_output_sample_format,
      ffmpeg_output_sample_rate_hz, ffmpeg_input_channel_layout,
      ffmpeg_input_sample_format, ffmpeg_input_sample_rate_hz, 0, nullptr);
  if (!swr_ctx_) {
    RTC_DCHECK(false);
    LOGE_TAG(kLogTag) << "failed to init audio converter, swr ctx is nullptr";
    return false;
  }

  if (swr_init(swr_ctx_) < 0) {
    RTC_DCHECK(false);
    LOGE_TAG(kLogTag)
        << "failed to init audio converter, swr ctx init failure.";
    return false;
  }

  inited_ = true;
  first_frame_ = true;
  LOGI_TAG(kLogTag) << "Init done, input param:" << in.ToString()
                    << ", output param:" << out.ToString();
  return true;
}

int AudioConverterFFmpeg::Convert(const AudioFrameLiteView &src_frame,
                                  AudioFrameLiteDelegate &dst_frame) {
  if (src_frame.FrameSize() == 0) {
    return 0;
  }
  if (src_frame.AudioFormat() == dst_frame.AudioFormat()) {
    dst_frame.CopyFrom(src_frame.Data(), src_frame.ByteSize());
    return 0;
  }

  ReinitIfNeeded(src_frame.AudioFormat(), dst_frame.AudioFormat());

  dst_frame.ResetReadableSizeInByte(
      src_frame.FormatConvertByteSize(dst_frame.AudioFormat()));

  // ptr_in or ptr_out, only the first one need be set in case of packed audio
  int res = swr_convert(swr_ctx_, dst_frame.MutablePlanarData().data(),
                        static_cast<int>(dst_frame.CapacityInFrames()), src_frame.PlanarData().data(),
                        static_cast<int>(src_frame.FrameSize()));
  if (res < 0) {
    RTC_DCHECK(false);
    LOGE_TAG(kLogTag)
        << "AudioConverterFFmpeg::Convert, failed to resample. error code: "
        << res;
    dst_frame.ResetReadableSizeInByte(0);
    return -1;
  }
  AdapterFfmpegOutputSamples(res, dst_frame);
  return 0;
}

AudioFrameLiteView AudioConverterFFmpeg::ConvertNoCopy(
    const AudioFrameLiteView &src_frame, AudioFrameLiteDelegate &dst_frame) {
  if (src_frame.FrameSize() == 0) {
    return dst_frame;
  }
  if (src_frame.AudioFormat() == dst_frame.AudioFormat()) {
    return src_frame;
  } else {
    Convert(src_frame, dst_frame);
    return dst_frame;
  }
}

void AudioConverterFFmpeg::UnInit() {
  if (swr_ctx_) {
    swr_free(&swr_ctx_);
    swr_ctx_ = nullptr;
  }
  inited_ = false;
}

void AudioConverterFFmpeg::AdapterFfmpegOutputSamples(
    int out_len, AudioFrameLiteDelegate &dst_frame) {
  RTC_DCHECK(first_frame_ ||
             (std::abs(out_len - static_cast<int>(dst_frame.FrameSize())) <= 1))
      << "unexpect out_len: " << out_len
      << " expect out_len is: " << dst_frame.FrameSize();

  if (static_cast<size_t>(out_len) < dst_frame.FrameSize()) {
    size_t len = out_len * dst_frame.AudioFormat().ByteSizePerFrame();
    size_t ch_count = dst_frame.AudioFormat().IsPlanner()
                          ? dst_frame.AudioFormat().GetChannelsCount()
                          : 1;
    uint8_t *ptr = dst_frame.MutableData();
    if (first_frame_ && first_frame_fill_zero_) {
      size_t data_len = len / ch_count;
      size_t offset = (dst_frame.ByteSize() - len) / ch_count;
      for (size_t i = 0; i < ch_count; ++i) {
        std::memmove(ptr + offset, ptr, data_len);
        memset(ptr, 0, offset);
        ptr += data_len + offset;
      }
    } else {
      // if out_len < dst_frame.FrameSize, Moving data makes it tightly packed.
      const uint8_t *dst = ptr;
      for (size_t i = 1; i < ch_count; ++i) {
        dst += dst_frame.ByteSize() / ch_count;
        ptr += len / ch_count;
        std::memmove(ptr, dst, len / ch_count);
      }
      dst_frame.ResetReadableSizeInByte(len);
    }
  }

  if (first_frame_) {
    first_frame_ = false;
  }
}

}  // namespace core
