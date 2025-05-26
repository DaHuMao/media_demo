#ifndef PLAYER_VIDEO_DECODE_TO_RENDER_H_
#define PLAYER_VIDEO_DECODE_TO_RENDER_H_
#include "api/sequence_checker.h"
#include "core/video_render/video_render.h"
#include "player/decode_to_render.h"
#include "player/player_timer.h"
namespace media_demo {
class VideoDecodeToRender final : public DecodeToRender {
 public:
  struct Config : public DecodeToRender::Config {
    core::VideoRender* video_render = nullptr;
    PlayerTimer* timer = nullptr;
  };
  VideoDecodeToRender() = default;
  ~VideoDecodeToRender() = default;
  int Init(const Config& config);
  int UnInit();

 private:
  int32_t OnFrame(const FrameInfo* frame) override;
  int64_t max_diff_ms_ = 15;
  Config config_;
  webrtc::SequenceChecker sequence_checker_;
  int64_t last_frame_pts_ms_ = 0;
  int32_t last_task_delay_ = 0;
};
}  // namespace media_demo
#endif  // PLAYER_VIDEO_DECODE_TO_RENDER_H_
