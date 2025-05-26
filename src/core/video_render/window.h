#ifndef CORE_VIDEO_RENDER_WINDOW_H_
#define CORE_VIDEO_RENDER_WINDOW_H_
#include "core/video_common/video_format_define.h"
namespace core {
class Window {
 public:
  virtual ~Window() = default;
  virtual bool Bind() = 0;
  virtual VideoSize GetSize() = 0;
  virtual int ReCreateSuface() = 0;
  virtual void SwapBuffers() = 0;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_WINDOW_H_
