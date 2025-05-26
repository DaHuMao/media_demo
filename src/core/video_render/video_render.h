#ifndef CORE_VIDEO_RENDER_H_
#define CORE_VIDEO_RENDER_H_
#include "core/video_common/video_frame.h"
#include "core/video_render//window.h"
namespace core {
class VideoRender {
 public:
  virtual ~VideoRender() = default;
  virtual int Init(core::Window* windows) = 0;
  virtual int SendFrame(const core::VideoFrameDelegate& frame) = 0;
  virtual int Play() = 0;
  virtual int Pause() = 0;
  virtual int UnInit() = 0;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_H_
