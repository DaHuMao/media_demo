#include "core/video_render/opengl/vertex_index_buffer.h"
#include "GL/glew.h"
#include "core/video_render/opengl/gl_util.h"
namespace core {
VerTexIndexBuffer::VerTexIndexBuffer(const uint32_t* data, size_t count)
    : ibo_ele_count_(static_cast<uint32_t>(count)) {
    static_assert(sizeof(uint32_t) == sizeof(GLuint), "error size GLuint");
    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);

}

VerTexIndexBuffer::~VerTexIndexBuffer() {
    glDeleteBuffers(1, &ibo_);
}

void VerTexIndexBuffer::Bind() const {
    RUN_FUNC_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_));
}

void VerTexIndexBuffer::UnBind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
}  // namespace core

