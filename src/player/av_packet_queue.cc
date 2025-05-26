#include "player/av_packet_queue.h"
namespace media_demo {
AvPacketQueue::AvPacketQueue(util::MillisecondsClass reserved_time)
    : audio_packet_queue_(reserved_time, false),
      video_packet_queue_(reserved_time, true) {}

void AvPacketQueue::PushPacket(PacketInfo& packet, bool is_video) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_video) {
    video_packet_queue_.PushPacket(packet);
  } else {
    audio_packet_queue_.PushPacket(packet);
  }
}

int64_t AvPacketQueue::GetDuration(bool is_video) {
  std::lock_guard<std::mutex> lock(mutex_);
  return is_video ? video_packet_queue_.GetDuration() :
    audio_packet_queue_.GetDuration();
}

const PacketInfo& AvPacketQueue::PeekPacket(bool is_video) {
  std::lock_guard<std::mutex> lock(mutex_);
  return is_video ? video_packet_queue_.PeekPacket()
                  : audio_packet_queue_.PeekPacket();
}

core::FfmpegStatus AvPacketQueue::GetPacket(PacketProxy* packet, bool is_video) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_video) {
    return video_packet_queue_.GetPacket(packet);
  } else {
    return audio_packet_queue_.GetPacket(packet);
  }
}

bool AvPacketQueue::SeekTo(int64_t target_ms, uint32_t serial) {
  std::lock_guard<std::mutex> lock(mutex_);
  return audio_packet_queue_.SeekTo(target_ms, serial) &&
         video_packet_queue_.SeekTo(target_ms, serial);
}

void AvPacketQueue::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  audio_packet_queue_.Clear();
  video_packet_queue_.Clear();
}

void AvPacketQueue::SetEof() {
  std::lock_guard<std::mutex> lock(mutex_);
  audio_packet_queue_.SetEof();
  video_packet_queue_.SetEof();
}

}  // namespace media_demo
