#include "player/player_timer.h"
#include "util/time_to_class.h"
namespace media_demo {

bool InValidTime(int64_t time) {
  return time == std::numeric_limits<int64_t>::min();
}

void PlayerTimer::SetAudioCurrentTime(int64_t current_time_ms, int32_t seek_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (type_ == PlayerTimerType::kAudio && seek_id == seek_id_) {
    last_update_time_ms_ = util::TimeNow().Value();
    last_render_pts_ms_ = current_time_ms;
  }
}

void PlayerTimer::SetVideoCurrentTime(int64_t current_time_ms, int32_t seek_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (type_ == PlayerTimerType::kVideo&& seek_id == seek_id_) {
    last_update_time_ms_ = util::TimeNow().Value();
    last_render_pts_ms_ = current_time_ms;
  }
}

void PlayerTimer::SeekTo(int64_t current_time_ms) {
  std::lock_guard<std::mutex> lock(mutex_);
  ++seek_id_;
}

void PlayerTimer::Seek(int64_t diff_time_ms) {
  std::lock_guard<std::mutex> lock(mutex_);
  ++seek_id_;
  last_update_time_ms_ = util::TimeNow().Value();
  last_render_pts_ms_ += diff_time_ms;
}

int64_t PlayerTimer::GetCurrentMainTime() {
  std::lock_guard<std::mutex> lock(mutex_);
  int64_t now = util::TimeNow().Value();
  if (last_update_time_ms_ == 0 || is_paused_) {
    last_update_time_ms_ = now;
  }
  return now - last_update_time_ms_ + last_render_pts_ms_;
}

void PlayerTimer::Pause() {
  std::lock_guard<std::mutex> lock(mutex_);
  is_paused_ = true;
}

void PlayerTimer::Resume() {
  std::lock_guard<std::mutex> lock(mutex_);
  is_paused_ = false;
}

}  // namespace media_demo
