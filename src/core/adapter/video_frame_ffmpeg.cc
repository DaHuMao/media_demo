#include "core/adapter/video_frame_ffmpeg.h"

#include "core/adapter/video_format_converter.h"
#include "core/video_common/video_format_util.h"
namespace core {
const VideoFrameDelegate VideoFrameFfmpeg::GetVideoFrame(
    const core::AvFrameWrapper* frame_wrapper) {
  auto frame = frame_wrapper->GetFrame();
  RawVideoFormat format =
      ToRawVideoFormat(static_cast<AVPixelFormat>(frame->format));
  VideoSize size = {frame->width, frame->height};
  int plane_num = GetPlaneNum(format);
  std::vector<int> strides(plane_num);
  std::vector<uint8_t*> data(plane_num);
  for (int i = 0; i < plane_num; i++) {
    strides[i] = frame->linesize[i];
    data[i] = frame->data[i];
  }
  return VideoFrameDelegate(data, VideoFormatInfo(size, format, strides),
                            frame_wrapper->GetPtsMs());
}
}  // namespace core
