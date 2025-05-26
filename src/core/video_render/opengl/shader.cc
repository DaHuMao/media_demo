#include "shader.h"

#include <fstream>
#include <sstream>
#include <string>

#include "GL/glew.h"
#include "core/video_render/opengl/gl_util.h"
#include "rtc_base/checks.h"
#include "util/log.h"
namespace core {
constexpr char kLogTag[] = "Shader";
static std::string ParseShader(const std::string& file_path) {
  std::ifstream ifs(file_path);
  if (!ifs) {
    RTC_DCHECK(false) << "open file failed: " << file_path;
    return "";
  }
  std::stringstream ss;
  std::string line;
  while (getline(ifs, line)) {
    ss << line << '\n';
  }
  return ss.str();
}

Shader::Shader(const std::string& vertex_shader,
               const std::string& fragment_shader, bool is_file_path) {
  // std::cout << vertex_shader_ << '\n' << fragment_shader_ << '\n';
  if (is_file_path) {
    vertex_shader_ = ParseShader(vertex_shader);
    fragment_shader_ = ParseShader(fragment_shader);
  } else {
    vertex_shader_ = vertex_shader;
    fragment_shader_ = fragment_shader;
  }
  RTC_DCHECK(!vertex_shader_.empty() && !fragment_shader_.empty());
  if ("" != vertex_shader_ && "" != fragment_shader_) {
    render_id_ = CreateShader(vertex_shader_, fragment_shader_);
  }
}

Shader::~Shader() {
  if (0 != render_id_) {
    RUN_FUNC_CHECK(glDeleteProgram(render_id_));
  }
}

void Shader::Bind() const { RUN_FUNC_CHECK(glUseProgram(render_id_)); }

void Shader::UnBind() const { glUseProgram(0); }

void Shader::CompileShader() {}

void Shader::SetUniform1i(const std::string& name, int value) {
  RUN_FUNC_CHECK(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value) {
  RUN_FUNC_CHECK(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2,
                          float v3) {
  RUN_FUNC_CHECK(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1,
                          float v2) {
  RUN_FUNC_CHECK(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform3f(const std::string& name,
                          const MatUtil::Coordinate3D& vec3) {
  SetUniform3f(name, vec3.x, vec3.y, vec3.z);
}

void Shader::SetUniform4fv(const std::string& name, float* ptr) {
  RUN_FUNC_CHECK(
      glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, ptr));
}

int Shader::CreateShader(const std::string& vertexShader,
                         const std::string& fragmentShader) {
  uint32_t program = glCreateProgram();
  uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
  uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
  RUN_FUNC_CHECK(glAttachShader(program, vs));
  RUN_FUNC_CHECK(glAttachShader(program, fs));
  glLinkProgram(program);
  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }
  glValidateProgram(program);
  glDeleteProgram(vs);
  glDeleteProgram(fs);
  return program;
}

uint32_t Shader::CompileShader(uint32_t type, const std::string source) {
  uint32_t id = glCreateShader(type);
  const char* str = source.c_str();
  glShaderSource(id, 1, &str, nullptr);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (GL_FALSE == result) {
    int len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
    char* message = (char*)alloca(len * sizeof(char));
    glGetShaderInfoLog(id, len, &len, message);
    LOGE_TAG(kLogTag) << "failed to compile "
                      << (GL_VERTEX_SHADER == type ? "vertex：" : "fragment: ")
                      << message;
    glDeleteShader(id);
    return 0;
  }
  return id;
}

int32_t Shader::GetUniformLocation(const std::string& name) {
  auto it = uniform_cache_.find(name);
  if (uniform_cache_.end() != it) {
    return it->second;
  } else {
    int ret = glGetUniformLocation(render_id_, name.c_str());
    if (ret < 0) {
      LOGE_TAG(kLogTag) << "glGetUniformLocation error, name: " << name
                        << " ret: " << ret;
    }
    uniform_cache_[name] = ret;
    return ret;
  }
}
}  // namespace core
