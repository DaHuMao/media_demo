
#include "core/ffmpeg/av_packet_wrapper.h"
#include "rtc_base/checks.h"
namespace core {
AvPacketWrapper::AvPacketWrapper() : packet_(av_packet_alloc()) {}
AvPacketWrapper::AvPacketWrapper(AvPacketWrapper&& oth) {
  time_base_ = oth.time_base_;
  packet_ = oth.packet_;
  oth.packet_ = nullptr;
}

AvPacketWrapper& AvPacketWrapper::operator=(AvPacketWrapper&& oth) {
  if (this != &oth) {
    time_base_ = oth.time_base_;
    if (packet_ != nullptr) {
      av_packet_free(&packet_);
    }
    packet_ = oth.packet_;
    oth.packet_ = nullptr;
  }
  return *this;
}

AvPacketWrapper::~AvPacketWrapper() {
  if (packet_ != nullptr) {
    av_packet_free(&packet_);
  }
}
void AvPacketWrapper::MoveFromPaket(AvPacketWrapper& oth) {
  time_base_ = oth.time_base_;
  av_packet_unref(packet_);
  av_packet_move_ref(packet_, oth.GetPacket());
  return;
}

void AvPacketWrapper::UnRefPacket() {
  av_packet_unref(packet_);
  return;
}

bool AvPacketWrapper::IsValid() {
  return packet_ != nullptr && packet_->data != nullptr &&
    time_base_.num != 0 && time_base_.den != 0;
}

void AvPacketWrapper::SetTimeBase(AVRational time_base) {
  RTC_DCHECK(time_base.num != 0 && time_base.den != 0);
  time_base_ = time_base;
}

int64_t AvPacketWrapper::GetPtsMs() {
  if (!IsValid()) {
    RTC_DCHECK(false) << "packet is invalid";
    return std::numeric_limits<int64_t>::min();
  }
  if (time_base_.num == 0 || time_base_.den == 0) {
    RTC_DCHECK(false) << "time base is invalid";
    return std::numeric_limits<int64_t>::min();
  }
  return av_rescale_q(GetPacket()->pts, time_base_, {1, 1000});
}

int64_t AvPacketWrapper::GetDurationMs() {
  if (!IsValid()) {
    RTC_DCHECK(false) << "packet is invalid";
    return std::numeric_limits<int64_t>::min();
  }
  if (time_base_.num == 0 || time_base_.den == 0) {
    RTC_DCHECK(false) << "time base is invalid";
    return std::numeric_limits<int64_t>::min();
  }
  return av_rescale_q(GetPacket()->duration, time_base_, {1, 1000});
}

}  // namespace core
