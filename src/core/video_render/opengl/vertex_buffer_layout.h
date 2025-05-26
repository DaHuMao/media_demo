#ifndef CORE_VIDEO_RENDER_OPENGL_VERTEX_BUFFER_LAYOUT_H_
#define CORE_VIDEO_RENDER_OPENGL_VERTEX_BUFFER_LAYOUT_H_
#include <vector>

#include "GL/glew.h"
#include "rtc_base/checks.h"
namespace core {
struct VerTexBufferElement {
  uint32_t type = 0;
  uint32_t count = 0;
  bool normalize = false;
  static uint32_t GetSizeOfType(uint32_t type) {
    switch (type) {
      case GL_FLOAT:
        return 4;
      case GL_UNSIGNED_INT:
        return 4;
      case GL_UNSIGNED_BYTE:
        return 1;
    }
    RTC_DCHECK(false) << "error type" << type;
    return 0;
  }
};

class VerTexBufferLayout {
 public:
  VerTexBufferLayout() = default;
  ~VerTexBufferLayout() = default;
  template <typename T>
  VerTexBufferLayout& Push(uint32_t count) {
    static_assert(false);
  }

  template <>
  VerTexBufferLayout& Push<uint32_t>(uint32_t count) {
    element_.push_back({GL_UNSIGNED_INT, count, false});
    stride_ += count * VerTexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
    return *this;
  }

  template <>
  VerTexBufferLayout& Push<float>(uint32_t count) {
    element_.push_back({GL_FLOAT, count, false});
    stride_ += count * VerTexBufferElement::GetSizeOfType(GL_FLOAT);
    return *this;
  }

  template <typename T>
  VerTexBufferLayout& Push(const std::vector<uint32_t>& count_arr) {
    for (uint32_t count : count_arr) {
      Push<T>(count);
    }
    return *this;
  }

  inline const std::vector<VerTexBufferElement>& GetElement() const {
    return element_;
  }
  inline uint32_t GetStride() const { return stride_; }

 private:
  std::vector<VerTexBufferElement> element_;
  int32_t stride_ = 0;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_VERTEX_BUFFER_LAYOUT_H_
