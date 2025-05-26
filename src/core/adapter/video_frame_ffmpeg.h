#ifndef CORE_ADAPTER_VIDEO_FRAME_FFMPEG_H_
#define CORE_ADAPTER_VIDEO_FRAME_FFMPEG_H_
#include "core/ffmpeg/av_frame_wrapper.h"
#include "core/video_common/video_frame.h"
namespace core {
class VideoFrameFfmpeg {
 public:
  static const VideoFrameDelegate GetVideoFrame(const AvFrameWrapper* frame);
};
}  // namespace core
#endif  // CORE_ADAPTER_VIDEO_FRAME_FFMPEG_H_
