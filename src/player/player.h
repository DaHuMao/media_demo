#ifndef PLAYER_PLAYER_H_
#define PLAYER_PLAYER_H_
#include <cstdint>
#include <memory>
#include <string>

#include "webrtc/api/task_queue/task_queue_factory.h"

#include "core/video_common/video_format_define.h"
#include "core/video_render//window.h"
#include "core/video_render/video_render_factory.h"
#include "util/point_delegate.h"
namespace media_demo {
class PlayerCallback {
 public:
  virtual ~PlayerCallback() = default;
  virtual void OnPrepared() = 0;
  virtual void OnCompleted() = 0;
  virtual void OnError(int32_t error_code, const std::string& error_msg) = 0;
  virtual void OnBufferingStart() = 0;
  virtual void OnBufferingEnd() = 0;
  virtual void OnSeekCompleted(int64_t target_ms, int64_t seek_id) = 0;
};
class Player {
 public:
  struct Config {
    std::string url;
    bool disable_audio = false;
    bool disable_video = false;
    PlayerCallback* callback = nullptr;
    core::Window* window = nullptr;
    core::VideoSize window_size;
  };
  struct PlayComponent {
    // 如果render为nullptr，则使用VideoRenderFactory构造一个render
    util::PointDelegate<core::VideoRender> video_render = nullptr;
  };
  virtual ~Player() = default;
  virtual int Init(const Config& config, PlayComponent component) = 0;
  virtual int Play() = 0;
  virtual int Pause() = 0;
  virtual int Seek(int64_t diff_current) = 0;
  virtual int SeekTo(int64_t target_ms) = 0;
  virtual int UnInit() = 0;
  virtual bool IsPlaying() = 0;
  virtual int64_t GetCurPositionMs() = 0;
  virtual int64_t GetDurationMs() = 0;
  static std::unique_ptr<Player> Create(
      util::PointDelegate<webrtc::TaskQueueFactory> task_queue_factory,
      std::unique_ptr<core::VideoRenderFactory> video_render_factory = nullptr);
};
}  // namespace media_demo
#endif  // PLAYER_PLAYER_H_
