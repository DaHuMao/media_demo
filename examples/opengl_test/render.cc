#include "render.h"

#include "core/video_render/opengl/gl_util.h"

void CustomRender::Draw(const core::VerTexArray& vao, const core::VerTexIndexBuffer& ib,
    const core::Shader& shader) const {
  vao.Bind();
  RUN_FUNC_CHECK(
      glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
  vao.UnBind();
}

void CustomRender::Clear() const { glClear(GL_COLOR_BUFFER_BIT); }
