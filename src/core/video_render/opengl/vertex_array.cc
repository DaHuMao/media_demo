#include "core/video_render/opengl/vertex_array.h"
#include "core/video_render/opengl/gl_util.h"

namespace core {
VerTexArray::VerTexArray() { glGenVertexArrays(1, &vao_); }

VerTexArray::~VerTexArray() { glDeleteVertexArrays(1, &vao_); }

void VerTexArray::AddBuffer(const VerTexBufferLayout& layout) {
  Bind();
  const auto& elements = layout.GetElement();
  uint32_t offset = 0;
  for (uint32_t i = 0; i < elements.size(); ++i) {
    const auto& element = elements[i];
    RUN_FUNC_CHECK(glVertexAttribPointer(i, element.count, element.type, element.normalize,
                          layout.GetStride(),
                          reinterpret_cast<const void*>(offset)));
    RUN_FUNC_CHECK(glEnableVertexAttribArray(i));
    offset += element.count * VerTexBufferElement::GetSizeOfType(element.type);
  }
}

void VerTexArray::Bind() const {
  RUN_FUNC_CHECK(glBindVertexArray(vao_));
}

void VerTexArray::UnBind() const { glBindVertexArray(0); }
}  // namespace core
