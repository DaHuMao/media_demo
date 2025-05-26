#include "core/video_render/opengl/mat_util.h"

#include <cmath>

#include "third_party/glm/gtc/matrix_transform.hpp"
#include "third_party/glm/gtc/type_ptr.hpp"
#include "third_party/glm/trigonometric.hpp"
namespace core {
namespace MatUtil {

std::ostream& operator<<(std::ostream& os, const Coordinate2D& coord) {
  os << "(" << coord.x << ", " << coord.y << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Coordinate3D& coord) {
  os << "(" << coord.x << ", " << coord.y << ", " << coord.z << ")";
  return os;
}
glm::vec3 GetVec3(const vec3& vec) { return glm::vec3(vec.x, vec.y, vec.z); }

void Normalize(Coordinate3D& vec) {
  float sum = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
  vec.x = vec.x / sum;
  vec.y = vec.y / sum;
  vec.z = vec.z / sum;
}

glm::vec3 RotateVec(glm::vec3 rotate_vec, glm::vec3 rotate_axis, float angle) {
  const float rad = glm::radians(angle);
  const float c = glm::cos(rad);
  const float s = glm::sin(rad);
  const glm::vec3& v = rotate_vec;
  return c * v + (1 - c) * glm::dot(v, rotate_axis) * v +
         s * glm::cross(v, rotate_axis);
}

glm::mat4 GetDefaultProjection(int width, int height) {
  return glm::perspective(glm::radians(45.f), width * 1.f / height, 0.1f,
                          100.0f);
}

Mat4& Mat4::TranslateLeft(const vec3& pos_diff) {
  mat_[3] += glm::vec4(pos_diff.x, pos_diff.y, pos_diff.z, 0);
  return *this;
}

Mat4& Mat4::Translate(const vec3& pos_diff) {
  mat_ = glm::translate(mat_, GetVec3(pos_diff));
  return *this;
}

Mat4& Mat4::RotateLeft(float angle, const vec3& axisv) {
  Rotate(angle, axisv);
  float const a = angle;
  float const c = cos(a);
  float const s = sin(a);

  auto v = GetVec3(axisv);
  glm::vec<3, float, glm::defaultp> axis(normalize(v));
  glm::vec<3, float, glm::defaultp> temp((1.0f - c) * axis);

  glm::mat4 rotate(1.0f);
  rotate[0][0] = c + temp[0] * axis[0];
  rotate[0][1] = temp[0] * axis[1] + s * axis[2];
  rotate[0][2] = temp[0] * axis[2] - s * axis[1];

  rotate[1][0] = temp[1] * axis[0] - s * axis[2];
  rotate[1][1] = c + temp[1] * axis[1];
  rotate[1][2] = temp[1] * axis[2] + s * axis[0];

  rotate[2][0] = temp[2] * axis[0] + s * axis[1];
  rotate[2][1] = temp[2] * axis[1] - s * axis[0];
  rotate[2][2] = c + temp[2] * axis[2];

  auto m = mat_;
  mat_[0] = rotate[0] * m[0][0] + rotate[1] * m[0][1] + rotate[2] * m[0][2] +
            rotate[3] * m[0][3];
  mat_[1] = rotate[0] * m[1][0] + rotate[1] * m[1][1] + rotate[2] * m[1][2] +
            rotate[3] * m[1][3];
  mat_[2] = rotate[0] * m[2][0] + rotate[1] * m[2][1] + rotate[2] * m[2][2] +
            rotate[3] * m[2][3];
  mat_[3] = rotate[0] * m[3][0] + rotate[1] * m[3][1] + rotate[2] * m[3][2] +
            rotate[3] * m[3][3];
  return *this;
}

Mat4& Mat4::Rotate(float angle, const vec3& axis) {
  mat_ = glm::rotate(mat_, glm::radians(angle), GetVec3(axis));
  return *this;
}

Mat4& Mat4::Scale(const vec3& scale_vec) {
  mat_ = glm::scale(mat_, GetVec3(scale_vec));
  return *this;
}
float* Mat4::Ptr() { return glm::value_ptr(mat_); }

}  // namespace MatUtil
}  // namespace core
