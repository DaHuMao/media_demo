#ifndef RENDER_H_
#define RENDER_H_

#include "core/video_render/opengl/shader.h"
#include "core/video_render/opengl/vertex_array.h"
#include "core/video_render/opengl/vertex_index_buffer.h"

class CustomRender {
 public:
  CustomRender() = default;
  ~CustomRender() = default;
  void Draw(const core::VerTexArray& vao, const core::VerTexIndexBuffer& ib,
            const core::Shader& shader) const;
  void Clear() const;

 private:
};

#endif  // RENDER_H_
