#ifndef PLAYER_FRAME_INFO_H_
#define PLAYER_FRAME_INFO_H_
#include "core/ffmpeg/av_frame_wrapper.h"
namespace media_demo {
class FrameInfo : public core::AvFrameWrapper {
 public:
  FrameInfo(AVRational time_base) : AvFrameWrapper(time_base) {}
  ~FrameInfo() = default;
  void SetSerial(int32_t serial) { serial_ = serial; }
  int32_t GetSerial()const { return serial_; }
 private:
  int32_t serial_ = 0;
};
}  // namespace media_demo
#endif  // PLAYER_FRAME_INFO_H_
