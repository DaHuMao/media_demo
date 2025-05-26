#include "vertex_buffer.h"

#include "GL/glew.h"
namespace core {
VerTexBuffer::VerTexBuffer(const void* data, size_t size, bool is_static) {
  glGenBuffers(1, &vbo_);
  ResetData(data, size, is_static);
}

VerTexBuffer::~VerTexBuffer() { glDeleteBuffers(1, &vbo_); }
void VerTexBuffer::ResetData(const void* data, size_t size, bool is_static) {
  UnBind();
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  if (is_static) {
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  } else {
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
  }
}
void VerTexBuffer::Bind() const { glBindBuffer(GL_ARRAY_BUFFER, vbo_); }

void VerTexBuffer::UnBind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
}  // namespace core
