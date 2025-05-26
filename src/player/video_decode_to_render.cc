#include "player/video_decode_to_render.h"
#include "core/adapter/video_frame_ffmpeg.h"
namespace media_demo {
int VideoDecodeToRender::Init(const VideoDecodeToRender::Config& config) {
  if (is_init_) {
    return 0;
  }
  if (DecodeToRender::Init(config) != 0) {
    return -1;
  }
  if (config.video_render == nullptr || config.timer == nullptr) {
    is_init_ = false;
    return -1;
  }
  config_ = config;
  is_init_ = true;
  return 0;
}

int32_t VideoDecodeToRender::OnFrame(const FrameInfo* frame) {
  int task_delay = 0;
  auto diff =
      frame->GetPtsMs().Value() - config_.timer->GetCurrentMainTime();
  if (std::abs(diff) < max_diff_ms_) {
    if (config_.video_render->SendFrame(
            core::VideoFrameFfmpeg::GetVideoFrame(frame)) != 0) {
      task_delay = std::max(frame->GetDurationMs().Value(),
          static_cast<int64_t>(100));
    } else {
      config_.timer->SetVideoCurrentTime(frame->GetPtsMs().Value(),
          frame->GetSerial());
      last_frame_pts_ms_ = frame->GetPtsMs().Value();
      last_task_delay_ = 0;
    }
  } else {
    if (diff > 0) {
      task_delay = diff;
    }
  }
  last_task_delay_ += task_delay;
  return task_delay;
}
int VideoDecodeToRender::UnInit() {
  if (!is_init_) {
    return 0;
  }
  DecodeToRender::UnInit();
  is_init_ = false;
  return 0;
}

}  // namespace media_demo
