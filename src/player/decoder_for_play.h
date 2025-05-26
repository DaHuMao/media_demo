#ifndef PLAYER_DECODER_FOR_PLAY_H_
#define PLAYER_DECODER_FOR_PLAY_H_
#include "core/codec/decoder_ffmpeg.h"
#include "player/packet_info.h"
#include "core/adapter/audio_frame_ffmpeg.h"
#include "core/adapter/video_frame_ffmpeg.h"
namespace media_demo {
class DecoderForPlay {
 public:
  DecoderForPlay() = default;
  ~DecoderForPlay() = default;
  int Start(const std::string& url);
  int Flush();
  int Stop();
 private:
  bool decoder_is_init = false;
  int audio_stream_index = -1;
  int video_stream_index = -1;
  uint32_t serial = 0;
  int64_t video_packet_queue_duration_ = 0;
  int64_t audio_packet_queue_duration_ = 0;
  int64_t max_duration_ms = 0;
  AVStream* audio_stream = nullptr;
  AVStream* video_stream = nullptr;
  PacketInfo audio_packet;
  PacketInfo video_packet;
  core::AudioFrameFfmpeg audio_frame;
  core::VideoFrameFfmpeg video_frame;
  std::unique_ptr<core::DecoderFfmpeg> audio_decoder_;
  std::unique_ptr<core::DecoderFfmpeg> video_decoder_;
};
} // media_demo
#endif // PLAYER_DECODER_FOR_PLAY_H_
