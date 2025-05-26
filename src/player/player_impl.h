#ifndef PLAYER_PLAYER_IMPL_H_
#define PLAYER_PLAYER_IMPL_H_
#include <atomic>

#include "webrtc/api/task_queue/task_queue_factory.h"

#include "core/codec/reader_ffmpeg.h"
#include "core/video_render/video_render.h"
#include "player/audio_decode_to_render.h"
#include "player/audio_render.h"
#include "player/av_packet_queue.h"
#include "player/packet_info.h"
#include "player/player.h"
#include "player/player_define.h"
#include "player/player_timer.h"
#include "player/video_decode_to_render.h"
#include "rtc_base/thread.h"
#include "util/point_delegate.h"
namespace media_demo {

class PlayerImpl final : public Player {
 public:
  PlayerImpl(std::unique_ptr<core::VideoRenderFactory> video_render_factory,
             util::PointDelegate<webrtc::TaskQueueFactory> task_queue_factory);
  ~PlayerImpl() override;
  PlayerImpl(const PlayerImpl&) = delete;
  PlayerImpl& operator=(const PlayerImpl&) = delete;
  int Init(const Config& config, PlayComponent component) override;
  int Play() override;
  int Pause() override;
  bool IsPlaying() override;
  int Seek(int64_t diff_current) override;
  int SeekTo(int64_t target_ms) override;
  int UnInit() override;
  int64_t GetCurPositionMs() override;
  int64_t GetDurationMs() override;

  // AudioDecodeToRenderCallback
  void OnAudioCompleted();
  void OnAudioError();
  core::FfmpegStatus GetAudioPacket(PacketProxy& packet);

  // VideoDecodeToRenderCallback
  void OnVideoCompleted();
  void OnVideoError();
  core::FfmpegStatus GetVideoPacket(PacketProxy& packet);

 private:
  // 这个里面的变量都必须在player_task_queue_线程中访问
  struct PlayerThreadInfo {
    PlayerState state = PlayerState::kIdle;
    std::atomic_bool mute_audio = false;
    std::atomic_bool mute_video = false;
    std::atomic<int64_t> duration_ms = 0;
  };

  // 这个里面的变量都必须在reader_task_queue_线程中访问
  struct ReaderThreadInfo {
    bool reader_is_init = false;
    bool is_exit = false;
    int64_t seek_diff_current_ms = 0;
    std::atomic<int64_t> current_pos_ms = 0;
    uint32_t serial = 0;
    PacketInfo packet_;
    std::unique_ptr<core::ReaderFfmpeg> reader_;
  };

  void InitInternal(const Config& config, PlayComponent component);
  void PlayInternal();
  void PauseInternal();
  void SeekInternal(int64_t diff_current);
  void UnInitInternal();

  bool CheckState(PlayerState state);
  bool SetState(PlayerState state);
  PlayerState GetState();
  bool ThreadShouldExit();
  void ReadPacket();
  int InitReaderIfNeed();
  void UnInitReader();
  bool InitDecoder();
  void UnInitDecoder();
  void SetError(int32_t error_code, const std::string& error_msg);

  std::mutex mutex_;
  int id_ = 0;
  int64_t max_packet_queue_size_ms_ = 1000;
  Config config_;
  PlayerTimer timer_;
  PlayerThreadInfo player_info_;
  ReaderThreadInfo reader_info_;
  std::unique_ptr<rtc::Thread> player_task_queue_ = nullptr;
  std::unique_ptr<rtc::Thread> reader_task_queue_ = nullptr;
  std::unique_ptr<rtc::Thread> decoder_task_queue_ = nullptr;
  std::unique_ptr<AudioRender> audio_render_;
  std::unique_ptr<AudioDecodeToRender> audio_decode_to_render_;
  std::unique_ptr<VideoDecodeToRender> video_decode_to_render_;
  std::unique_ptr<core::VideoRenderFactory> video_render_factory_;
  util::PointDelegate<webrtc::TaskQueueFactory> task_queue_factory_;
  PlayComponent component_;
  AvPacketQueue av_packet_queue_;
  uint32_t seek_id_ = 0;
  int64_t start_time_ = 0;
};
}  // namespace media_demo
#endif  // PLAYER_PLAYER_IMPL_H_
