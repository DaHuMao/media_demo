#include "player/decode_to_render.h"
namespace media_demo {
DecodeToRender::DecodeToRender() : decoder_(new core::DecoderFfmpeg()) {}
int DecodeToRender::Init(const DecodeToRender::Config& config) {
  if (is_init_) {
    return 0;
  }
  if (config.stream == nullptr || config.on_error == nullptr ||
      config.on_completed == nullptr || config.task_queue == nullptr) {
    return -1;
  }
  config_ = config;
  av_frame_ = std::make_unique<FrameInfo>(config_.stream->time_base);
  decoder_->InitCodec(config_.stream, [this](core::AvPacketWrapper& packet) {
    return this->GetPacket(packet);
  });
  is_init_ = true;
  return 0;
}

int DecodeToRender::Process() {
  if (!is_init_) {
    return -1;
  }
  config_.task_queue->PostTask([this]() { this->ProcessInternal(); });
  return 0;
}

void DecodeToRender::ProcessInternal() {
  if (!is_init_) {
    return;
  }
  bool can_send = last_av_frame_pending_;
  int32_t task_delay = 0;
  if (!last_av_frame_pending_) {
    auto ret = decoder_->Receive(av_frame_->GetFrame());
    av_frame_->SetSerial(newest_packet_serial_);
    if (ret == core::FfmpegStatus::kOk) {
      last_av_frame_pending_ = true;
      can_send = true;
    } else if (ret == core::FfmpegStatus::kError) {
      config_.on_error();
      return;
    } else if (ret == core::FfmpegStatus::kEof) {
      config_.on_completed();
      return;
    } else if (ret == core::FfmpegStatus::kEAgain) {
      task_delay = 20;
    }
  }
  if (can_send && av_frame_->GetPtsMs().Value() >= start_time_) {
    task_delay = OnFrame(av_frame_.get());
  }
  if (task_delay < 0) {
    return;
  }
  if (task_delay > 0) {
    config_.task_queue->PostDelayedTask([this]() { this->ProcessInternal(); },
                                        webrtc::TimeDelta::Millis(task_delay));
  } else {
    last_av_frame_pending_ = false;
    config_.task_queue->PostTask([this]() { this->ProcessInternal(); });
  }
}

int DecodeToRender::UnInit() {
  is_init_ = false;
  serial_ = 0;
  return 0;
}

void DecodeToRender::SetSerial(uint32_t serial) {
  RTC_DCHECK_GT(serial, serial_);
  serial_ = serial;
}

core::FfmpegStatus DecodeToRender::GetPacket(core::AvPacketWrapper& packet) {
  auto status = core::FfmpegStatus::kOk;
  PacketProxy packet_proxy(&packet);
  bool should_flush_decoder = false;
  while (status == core::FfmpegStatus::kOk) {
    status = config_.get_packet(packet_proxy);
    if (status == core::FfmpegStatus::kOk) {
      if (packet_proxy.GetSerial() != serial_) {
        RTC_CHECK_LT(packet_proxy.GetSerial(), serial_);
        packet_proxy.UnRefPacket();
        should_flush_decoder = true;
      } else {
        break;
      }
    }
  }
  if (should_flush_decoder) {
    decoder_->Flush();
  }
  newest_packet_serial_ = packet_proxy.GetSerial();
  return status;
}

}  // namespace media_demo
