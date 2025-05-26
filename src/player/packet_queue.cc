#include "player/packet_queue.h"

#include "rtc_base/checks.h"
namespace media_demo {
PacketQueue::PacketQueue(util::MillisecondsClass reserved_time, bool is_video)
    : is_video_(is_video), reserved_time_(reserved_time) {
  packet_queue_.resize(10);
}

PacketQueue::~PacketQueue() {}

void PacketQueue::PushPacket(PacketInfo& packet) {
  if (!packet.IsValid()) {
    RTC_DCHECK(false) << "packet is invalid";
    return;
  }
  if (write_index_ == -1) {
    size_t size = packet_queue_.size();
    ReSizeQueue();
    write_index_ = static_cast<int>(size);
  }
  auto& packet_info = packet_queue_[write_index_];
  packet_info.MoveFromPaket(packet);
  if (read_index_ == -1) {
    read_index_ = write_index_;
  }
  last_writed_index_ = write_index_;
  ++write_index_;
  write_index_ = write_index_ % packet_queue_.size();
  if (write_index_ == first_reserved_index_ ||
      write_index_ == read_index_) {
    write_index_ = -1;
  }
}

int64_t PacketQueue::GetDuration() {
  int64_t last_dts = 0, first_dts = 0;
  if (read_index_ >= 0) {
    first_dts = packet_queue_[read_index_].GetPacket()->dts;
  }
  if (last_writed_index_ >= 0) {
    last_dts = packet_queue_[last_writed_index_].GetPacket()->dts;
  }
  return last_dts - first_dts;
}

const PacketInfo& PacketQueue::PeekPacket() {
  return packet_queue_[read_index_];
}

core::FfmpegStatus PacketQueue::GetPacket(PacketProxy* packet) {
  if (read_index_ != -1 && nullptr != packet) {
    auto& packet_info = packet_queue_[read_index_];
    if (!packet_info.IsValid()) {
      RTC_DCHECK(false) << "packet is invalid";
      return core::FfmpegStatus::kError;
    }
    packet->MoveFromPaket(packet_info);
    RTC_DCHECK(packet->IsValid());
    if (write_index_ == -1) {
      write_index_ = read_index_;
    }
    if (read_index_ == last_writed_index_) {
      last_writed_index_ = -1;
    }
    read_index_ = (read_index_ + 1) % packet_queue_.size();
    if (read_index_ == write_index_) {
      read_index_ = -1;
    }
    return core::FfmpegStatus::kOk;
  }
  return is_eof_ ? core::FfmpegStatus::kEof : core::FfmpegStatus::kEAgain;
}

bool PacketQueue::SeekTo(int64_t target_ms, uint32_t serial) { return false; }

void PacketQueue::Clear() {
  for (auto& packet : packet_queue_) {
    packet.UnRefPacket();
  }
  read_index_ = -1;
  write_index_ = 0;
}

void PacketQueue::ReSizeQueue() {
  std::vector<PacketInfo> new_vec;
  new_vec.resize(packet_queue_.size() * 2);
  for (size_t i = 0; i < packet_queue_.size(); ++i) {
    new_vec[i] = std::move(packet_queue_[i]);
  }
  packet_queue_ = std::move(new_vec);
}

void PacketQueue::HandleReservedPacket(int index) {
  RTC_CHECK_GE(first_reserved_index_, 0);
  auto packet = packet_queue_[index].GetPacket();
  if (is_video_) {
    if (packet->flags & AV_PKT_FLAG_KEY) {
      if (first_reserved_index_ != -1 && second_idr_index_ == -1) {
        second_idr_index_ = read_index_;
      }
      if (second_idr_index_ > 0 &&
          PacketDiffMs(index, second_idr_index_) > reserved_time_.Value()) {
        UnRefPacket(first_reserved_index_, second_idr_index_);
        first_reserved_index_ = second_idr_index_;
      }
    }
  } else {
    for (int i = first_reserved_index_; i < index; i++) {
      auto diff_ms = PacketDiffMs(index, i);
      if (diff_ms > reserved_time_.Value()) {
        packet_queue_[i].UnRefPacket();
        first_reserved_index_ = i + 1;
      }
    }
  }
}

int64_t PacketQueue::PacketDiffMs(int index1, int index2) {
  RTC_CHECK_GE(packet_queue_[index1].GetPacket()->dts,
               packet_queue_[index2].GetPacket()->dts);
  return av_rescale_q(packet_queue_[index1].GetPacket()->dts -
                          packet_queue_[index2].GetPacket()->dts,
                      {1, AV_TIME_BASE}, {1, 1000});
}

void PacketQueue::UnRefPacket(int start, int end) {
  while (start != end) {
    packet_queue_[start].UnRefPacket();
    start = (start + 1) % packet_queue_.size();
  }
}

}  // namespace media_demo
