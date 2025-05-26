#include "core/ffmpeg/av_frame_wrapper.h"
#include "rtc_base/checks.h"
namespace core {
AvFrameWrapper::AvFrameWrapper(AVRational time_base)
    : frame_(av_frame_alloc()), time_base_(time_base) {
  RTC_CHECK(frame_ != nullptr);
}
AvFrameWrapper::AvFrameWrapper()
    : frame_(av_frame_alloc()) {}
AvFrameWrapper::~AvFrameWrapper() {
  if (frame_) {
    av_frame_free(&frame_);
  }
}
AvFrameWrapper::AvFrameWrapper(AvFrameWrapper&& other) : frame_(other.frame_) {
  other.frame_ = nullptr;
}

AvFrameWrapper& AvFrameWrapper::operator=(AvFrameWrapper&& other) {
  if (this != &other) {
    if (frame_) {
      av_frame_free(&frame_);
    }
    frame_ = other.frame_;
    other.frame_ = nullptr;
  }
  return *this;
}

static bool IsInValidTimeBase(const AVRational& time_base) {
  return time_base.num == 0 && time_base.den == 0;
}

util::MillisecondsClass AvFrameWrapper::GetPtsMs() const {
  if (IsInValidTimeBase(time_base_)) {
    RTC_DCHECK(false) << "invalid time base";
    return util::MillisecondsClass(0);
  }
  return util::MillisecondsClass(
      av_rescale_q(frame_->pts, time_base_, {1, 1000}));
}

util::MillisecondsClass AvFrameWrapper::GetDtsMs() const {
  if (IsInValidTimeBase(time_base_)) {
    RTC_DCHECK(false) << "invalid time base";
  }
  return util::MillisecondsClass(
      av_rescale_q(frame_->pkt_dts, time_base_, {1, 1000}));
}

util::MillisecondsClass AvFrameWrapper::GetDurationMs() const {
  if (IsInValidTimeBase(time_base_)) {
    RTC_DCHECK(false) << "invalid time base";
  }
  return util::MillisecondsClass(
      av_rescale_q(frame_->pkt_duration, time_base_, {1, 1000}));
}

}  // namespace core
