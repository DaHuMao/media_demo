#ifndef CORE_FFMPEG_AV_FRAME_WRAPPER_H_
#define CORE_FFMPEG_AV_FRAME_WRAPPER_H_
#include "libavutil/rational.h"
#include "util/time_to_class.h"
extern "C" {
#include "libavutil/frame.h"
}
namespace core {
class AvFrameWrapper {
 public:
  AvFrameWrapper(AVRational time_base);
  AvFrameWrapper();
  virtual ~AvFrameWrapper();
  AvFrameWrapper(AvFrameWrapper&&);
  AvFrameWrapper& operator=(AvFrameWrapper&&);
  AVFrame* GetFrame() { return frame_; }
  const AVFrame* GetFrame() const { return frame_; }
  util::MillisecondsClass GetPtsMs() const;
  util::MillisecondsClass GetDtsMs() const;
  util::MillisecondsClass GetDurationMs() const;
 protected:
  AVFrame* frame_ = nullptr;
  AVRational time_base_ = {0, 0};
};
} // namespace core
#endif // CORE_FFMPEG_AV_FRAME_WRAPPER_H_
