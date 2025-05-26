#ifndef CORE_VIDEO_VIDEO_COMMON_CONVERT_H_
#define CORE_VIDEO_VIDEO_COMMON_CONVERT_H_
#include "core/video_common/video_frame.h"
namespace core {
class VideoConvert final {
 public:
  int Convert(const VideoFrameDelegate& frame, VideoFrameDelegate& out_frame);
};
}  // namespace core
#endif  // CORE_VIDEO_VIDEO_COMMON_CONVERT_H_
