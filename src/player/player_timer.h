#ifndef PLAYER_PLAYER_TIMER_H_
#define PLAYER_PLAYER_TIMER_H_
#include <cstdint>
#include <mutex>
namespace media_demo {
enum class PlayerTimerType {
  kAudio,
  kVideo,
};

class PlayerTimer {
 public:
  PlayerTimer(PlayerTimerType type, int init_seek_id = 0)
      : seek_id_(init_seek_id), type_(type) {}
  ~PlayerTimer() = default;
  void SetAudioCurrentTime(int64_t current_time_ms, int32_t seek_id);
  void SetVideoCurrentTime(int64_t current_time_ms, int32_t seek_id);
  void SeekTo(int64_t current_time_ms);
  void Seek(int64_t diff_time_ms);
  void Pause();
  void Resume();
  int64_t GetCurrentMainTime();

 private:
  bool is_paused_ = false;
  int32_t seek_id_ = 0;
  const PlayerTimerType type_;
  int64_t last_render_pts_ms_ = 0;
  int64_t last_update_time_ms_ = 0;
  std::mutex mutex_;
};
}  // namespace media_demo
#endif  // PLAYER_PLAYER_TIMER_H_
