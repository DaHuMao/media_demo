#ifndef CORE_VIDEO_RENDER_OPENGL_SHADER_H_
#define CORE_VIDEO_RENDER_OPENGL_SHADER_H_
#include <string>
#include <unordered_map>

#include "core/video_render/opengl/mat_util.h"
namespace core {
class Shader {
 public:
  Shader(const std::string& vertex_shader, const std::string& fragment_shader,
         bool is_file_path = true);
  ~Shader();
  void Bind() const;
  void UnBind() const;

  // Set Uniform
  void SetUniform3f(const std::string& name, float v0, float v1, float v2);
  void SetUniform4f(const std::string& name, float v0, float v1, float v2,
                    float v3);
  void SetUniform1i(const std::string& name, int value);
  void SetUniform1f(const std::string& name, float value);
  void SetUniform4fv(const std::string& name, float* ptr);
  void SetUniform3f(const std::string& name, const MatUtil::Coordinate3D& vec3);

 private:
  int32_t GetUniformLocation(const std::string& name);
  void CompileShader();
  int CreateShader(const std::string& vertexShader,
                   const std::string& fragmentShader);
  uint32_t CompileShader(uint32_t type, const std::string source);

  uint32_t render_id_ = 0;
  std::string vertex_shader_;
  std::string fragment_shader_;
  std::unordered_map<std::string, int32_t> uniform_cache_;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_SHADER_H_
