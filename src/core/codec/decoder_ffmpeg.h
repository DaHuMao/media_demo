#ifndef CORE_CODEC_DECODER_FFMPEG_H_
#define CORE_CODEC_DECODER_FFMPEG_H_
#include <functional>
#include "core/ffmpeg/av_packet_wrapper.h"
#include "core/ffmpeg/common_define.h"
extern "C" {
#include "libavformat/avformat.h"
}
namespace core {
class DecoderFfmpeg final {
 public:
  DecoderFfmpeg() = default;
  ~DecoderFfmpeg();
  int InitCodec(AVStream* stream, std::function<FfmpegStatus(AvPacketWrapper&)> get_packet);
  FfmpegStatus Receive(AVFrame* frame);
  int Flush();
  int UnInit();

 private:
  void Clear();
  bool is_init_ = false;
  bool packet_pending_ = false;
  AvPacketWrapper packet_;
  std::function<FfmpegStatus(AvPacketWrapper&)> get_packet_;
  AVCodecContext* decoder_ctx_ = nullptr;
};
}  // namespace core
#endif  // CORE_CODEC_DECODER_FFMPEG_H_
