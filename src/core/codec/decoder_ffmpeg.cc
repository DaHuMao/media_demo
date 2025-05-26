#include "core/codec/decoder_ffmpeg.h"
namespace core {
DecoderFfmpeg::~DecoderFfmpeg() { UnInit(); }
int DecoderFfmpeg::InitCodec(AVStream* stream,
                             std::function<FfmpegStatus(AvPacketWrapper&)> get_packet) {
  if (stream == nullptr) {
    return -1;
  }
  int ret = -1;
  do {
    auto codecpar = stream->codecpar;
    auto codec = avcodec_find_decoder(codecpar->codec_id);
    decoder_ctx_ = avcodec_alloc_context3(codec);
    if (nullptr == decoder_ctx_) {
      break;
    }
    if (nullptr != codecpar) {
      if (avcodec_parameters_to_context(decoder_ctx_, codecpar) < 0) {
        break;
      }
    }
    decoder_ctx_->pkt_timebase = stream->time_base;
    if (avcodec_open2(decoder_ctx_, codec, nullptr) < 0) {
      break;
    }
    ret = 0;
  } while (0);
  if (ret != 0) {
    Clear();
  }
  is_init_ = true;
  get_packet_ = get_packet;
  return ret;
}

FfmpegStatus DecoderFfmpeg::Receive(AVFrame* frame) {
  if (!is_init_ || !get_packet_ || !frame) {
    return FfmpegStatus::kError;
  }
  while (true) {
    int ret = avcodec_receive_frame(decoder_ctx_, frame);
    if (ret == AVERROR(EAGAIN)) {
      AVPacket* send_packet = packet_.GetPacket();
      if (!packet_pending_) {
        auto res = get_packet_(packet_);
        if (res == FfmpegStatus::kEof) {
          send_packet = nullptr;
        } else if (res == FfmpegStatus::kError || res == FfmpegStatus::kEAgain) {
          return res;
        }
      }
      ret = avcodec_send_packet(decoder_ctx_, send_packet);
      if (ret == AVERROR(EAGAIN)) {
        packet_pending_ = true;
        return FfmpegStatus::kEAgain;
      } else if (ret < 0) {
        return FfmpegStatus::kError;
      }
      packet_.UnRefPacket();
      // send_packet 成功,packet_pending_必然是置为false
      packet_pending_ = false;
    } else if (ret == AVERROR_EOF) {
      return FfmpegStatus::kEof;
    } else if (ret < 0) {
      return FfmpegStatus::kError;
    } else if (ret == 0) {
      return FfmpegStatus::kOk;
    }
  }
}

int DecoderFfmpeg::Flush() {
  if (!is_init_) {
    return -1;
  }
  avcodec_flush_buffers(decoder_ctx_);
  return 0;
}

int DecoderFfmpeg::UnInit() {
  if (is_init_) {
    Clear();
  }
  return 0;
}

void DecoderFfmpeg::Clear() {
  if (nullptr != decoder_ctx_) {
    avcodec_close(decoder_ctx_);
    avcodec_free_context(&decoder_ctx_);
    decoder_ctx_ = nullptr;
  }
  return;
}

}  // namespace core
