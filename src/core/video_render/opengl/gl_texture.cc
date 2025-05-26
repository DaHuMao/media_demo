#include "core/video_render/opengl/gl_texture.h"

#include "GL/glew.h"
#include "core/video_render/opengl/gl_util.h"
#include "rtc_base/checks.h"
namespace core {
GlTexture::GlTexture() {
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  // 为当前绑定的纹理对象设置环绕、过滤方式
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

GlTexture::~GlTexture() { glDeleteTextures(1, &texture_id_); }

void GlTexture::Bind(core::RGBVideoFrameView frame, uint32_t slot) const {
  RTC_CHECK(frame.GetPtr() != nullptr);
  if (nullptr != frame.GetPtr()) {
    RUN_FUNC_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.GetSize().width,
                 frame.GetSize().height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 frame.GetPtr()));
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0 + slot);
    RUN_FUNC_CHECK(glBindTexture(GL_TEXTURE_2D, texture_id_));
  }
}

void GlTexture::UnBind() const { glBindTexture(GL_TEXTURE_2D, 0); }
}  // namespace core
