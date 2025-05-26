#ifndef WEBRTC_AUDIO_AUDIO_CONVERTER_H_
#define WEBRTC_AUDIO_AUDIO_CONVERTER_H_

#include "core/audio/audio_format_define.h"
#include "core/audio/audio_frame.h"

struct SwrContext;

namespace core {
class AudioConverterFFmpeg {
 public:
  // 重采样第一帧数据是小于输入数据数量的，first_frame_fill_zero
  // 表明是否需要再第一帧 首部填充0补足数据量
  AudioConverterFFmpeg(bool first_frame_fill_zero = false);
  virtual ~AudioConverterFFmpeg();
  int Convert(const AudioFrameLiteView &src_frame,
              AudioFrameLiteDelegate &dst_frame);
  // if in.audio_format_ == out.audio_format_ return in
  // else return out
  AudioFrameLiteView ConvertNoCopy(const AudioFrameLiteView &src_frame,
                                   AudioFrameLiteDelegate &dst_frame);

 private:
  bool ReinitIfNeeded(const AudioFormatInfo &in, const AudioFormatInfo &out);
  bool Init(const AudioFormatInfo &in, const AudioFormatInfo &out);
  void UnInit();

  void AdapterFfmpegOutputSamples(int out_len,
                                  AudioFrameLiteDelegate &dst_frame);

  AudioFormatInfo format_in_;
  AudioFormatInfo format_out_;
  SwrContext *swr_ctx_ = nullptr;
  bool inited_ = false;
  const bool first_frame_fill_zero_ = false;
  bool first_frame_ = true;
};
}  // namespace core

#endif  // WEBRTC_AUDIO_AUDIO_CONVERTER_H_
