#ifndef CORE_VIDEO_RENDER_OPENGL_VERTEX_BUFFER_H_
#define CORE_VIDEO_RENDER_OPENGL_VERTEX_BUFFER_H_
#include <stdint.h>

#include <cstddef>
namespace core {
class VerTexBuffer {
 public:
  VerTexBuffer(const void* data, size_t size, bool is_static = true);
  ~VerTexBuffer();
  void ResetData(const void* data, size_t size, bool is_static = true);
  void Bind() const;
  void UnBind() const;

 private:
  uint32_t vbo_ = 0;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_VERTEX_BUFFER_H_
