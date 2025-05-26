#ifndef PLAYER_PACKET_INFO_H_
#define PLAYER_PACKET_INFO_H_
#include "core/ffmpeg/av_packet_wrapper.h"
namespace media_demo {
class PacketProxy {
 public:
  PacketProxy(core::AvPacketWrapper* packet_);
  ~PacketProxy() = default;
  void MoveFromPaket(PacketProxy& oth);
  void UnRefPacket();
  uint32_t GetSerial() { return serial_; }
  void SetSerial(uint32_t serial) { serial_ = serial; }
  void SetTimeBase(AVRational time_base) { packet_->SetTimeBase(time_base); }
  int64_t GetPtsMs() { return packet_->GetPtsMs(); }
  int64_t GetDurationMs() { return packet_->GetDurationMs(); }
  bool IsValid() { return packet_ != nullptr && packet_->IsValid(); }

 protected:
  PacketProxy() = default;
  void SetPacket(core::AvPacketWrapper* packet);

 private:
  core::AvPacketWrapper* packet_ = nullptr;
  uint32_t serial_ = 0;
};

class PacketInfo final : public PacketProxy {
 public:
  PacketInfo();
  PacketInfo(PacketInfo&& oth);
  PacketInfo& operator=(PacketInfo&& oth);
  AVPacket* GetPacket() { return private_packet_.GetPacket(); }

 private:
  PacketInfo(PacketInfo&) = delete;
  core::AvPacketWrapper private_packet_;
};
}  // namespace media_demo
#endif  // PLAYER_PACKET_INFO_H_
