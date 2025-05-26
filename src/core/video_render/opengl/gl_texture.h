#ifndef CORE_VIDEO_RENDER_OPENGL_GL_TEXTURE_H_
#define CORE_VIDEO_RENDER_OPENGL_GL_TEXTURE_H_
#include <cstdint>

#include "core/video_common/video_frame.h"
namespace core {
class GlTexture {
 public:
  GlTexture();
  ~GlTexture();
  void Bind(core::RGBVideoFrameView frame, uint32_t slot = 0) const;
  void UnBind() const;

 private:
  uint32_t texture_id_ = 0;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_GL_TEXTURE_H_
