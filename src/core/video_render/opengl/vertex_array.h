#ifndef CORE_VIDEO_RENDER_OPENGL_VERTEX_ARRAY_H_
#define CORE_VIDEO_RENDER_OPENGL_VERTEX_ARRAY_H_
#include "core/video_render/opengl/vertex_buffer.h"
#include "core/video_render/opengl/vertex_buffer_layout.h"
namespace core {
class VerTexArray {
 public:
  VerTexArray();
  ~VerTexArray();
  void AddBuffer(const VerTexBufferLayout& layout);
  void Bind() const;
  void UnBind() const;

 private:
  uint32_t vao_ = 0;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_VERTEX_ARRAY_H_
