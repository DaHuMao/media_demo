#include "player/packet_info.h"

#include "rtc_base/checks.h"
namespace media_demo {
PacketProxy::PacketProxy(core::AvPacketWrapper* packet) : packet_(packet) {
  RTC_DCHECK(nullptr != packet_);
}

void PacketProxy::MoveFromPaket(PacketProxy& oth) {
  if (oth.packet_ != nullptr && nullptr != packet_) {
    packet_->MoveFromPaket(*oth.packet_);
  }
  serial_ = oth.GetSerial();
  oth.serial_ = 0;
}

void PacketProxy::UnRefPacket() {
  if (packet_ != nullptr) {
    packet_->UnRefPacket();
  }
}


void PacketProxy::SetPacket(core::AvPacketWrapper* packet) {
  RTC_DCHECK(packet != nullptr);
  packet_ = packet;
}

PacketInfo::PacketInfo() { SetPacket(&private_packet_); }
PacketInfo::PacketInfo(PacketInfo&& oth)
    : PacketProxy(oth), private_packet_(std::move(oth.private_packet_)) {
  SetPacket(&private_packet_);
}
PacketInfo& PacketInfo::operator=(PacketInfo&& oth) {
  if (this != &oth) {
    SetSerial(oth.GetSerial());
    private_packet_ = std::move(oth.private_packet_);
  }
  return *this;
}
}  // namespace media_demo
