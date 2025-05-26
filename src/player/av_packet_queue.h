#ifndef PLAYER_AV_PACKET_QUEUE_H_
#define PLAYER_AV_PACKET_QUEUE_H_
#include "player/packet_info.h"
#include "player/packet_queue.h"
#include "util/time_to_class.h"
namespace media_demo {
class AvPacketQueue {
 public:
  AvPacketQueue(util::MillisecondsClass reserved_time);
  int64_t GetDuration(bool is_video);
  void PushPacket(PacketInfo& packet, bool is_video);
  const PacketInfo& PeekPacket(bool is_video);
  core::FfmpegStatus GetPacket(PacketProxy* packet, bool is_video);
  bool SeekTo(int64_t target_ms, uint32_t serial);
  void SetEof();
  void Clear();
 private:
  std::mutex mutex_;
  PacketQueue audio_packet_queue_;
  PacketQueue video_packet_queue_;
};
}  // namespace media_demo
#endif // PLAYER_AV_PACKET_QUEUE_H_
