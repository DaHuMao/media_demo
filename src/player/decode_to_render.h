#ifndef PLAYER_DECODE_TO_RENDER_H_
#define PLAYER_DECODE_TO_RENDER_H_
#include <functional>
#include "api/task_queue/task_queue_base.h"
#include "core/codec/decoder_ffmpeg.h"
#include "core/ffmpeg/common_define.h"
#include "player/frame_info.h"
#include "player/packet_info.h"
extern "C" {
#include "libavformat/avformat.h"
}
namespace media_demo {
// 只是将一些公共的代码提取出来
class DecodeToRender {
 public:
  struct Config {
    AVStream* stream = nullptr;
    webrtc::TaskQueueBase* task_queue = nullptr;
    std::function<core::FfmpegStatus(PacketProxy&)> get_packet;
    std::function<void()> on_error;
    std::function<void()> on_completed;
  };
  DecodeToRender();
  virtual ~DecodeToRender() = default;
  int Init(const Config& config);
  int Process();
  int UnInit();
  void SetSerial(uint32_t serial);
  void SetStartTime(int64_t start_time) { start_time_ = start_time; }
 protected:
  // 返回值为需要等待的时间
  // 如果返回值为0，表示已经消费了frame
  // 如果返回值为正数，表示需要等待的时间
  // 如果返回值为负数，表示需要退出队列
  virtual int32_t OnFrame(const FrameInfo* frame) = 0;
  bool is_init_ = false;
 private:
  core::FfmpegStatus GetPacket(core::AvPacketWrapper& packet);
  void ProcessInternal();
  bool last_av_frame_pending_ = false;
  uint32_t serial_ = 0;
  uint32_t newest_packet_serial_ = 0;
  int64_t start_time_ = 0;
  Config config_;
  std::unique_ptr<FrameInfo> av_frame_;
  std::unique_ptr<core::DecoderFfmpeg> decoder_;
};
}  // namespace media_demo
#endif  // PLAYER_DECODE_TO_RENDER_H_
