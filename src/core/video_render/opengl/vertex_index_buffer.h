#ifndef CORE_VIDEO_RENDER_OPENGL_INDEX_BUFFER_H_
#define CORE_VIDEO_RENDER_OPENGL_INDEX_BUFFER_H_
#include <stdint.h>

#include <cstddef>
namespace core {
class VerTexIndexBuffer {
 public:
  VerTexIndexBuffer(const uint32_t* data, size_t count);
  ~VerTexIndexBuffer();
  void Bind() const;
  void UnBind() const;
  inline uint32_t GetCount() const { return ibo_ele_count_; }

 private:
  uint32_t ibo_ = 0;
  uint32_t ibo_ele_count_ = 0;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_INDEX_BUFFER_H_
