#ifndef CORE_VIDEO_RENDER_OPENGL_MAT_UTIL_H_
#define CORE_VIDEO_RENDER_OPENGL_MAT_UTIL_H_
#include <iostream>

#include "third_party/glm/ext/matrix_float4x4.hpp"
#include "third_party/glm/ext/vector_float3.hpp"
namespace core {
namespace MatUtil {
struct Coordinate2D {
  float x = 0, y = 0;
  constexpr Coordinate2D() = default;
  constexpr Coordinate2D(float _x, float _y) : x(_x), y(_y) {}
  float* Ptr() { return &x; }
  bool operator==(const Coordinate2D& oth) { return x == oth.x && y == oth.y; }
  bool operator!=(const Coordinate2D& oth) { return !(*this == oth); }
};

struct Coordinate3D : public Coordinate2D {
  float z = 0;
  constexpr Coordinate3D() = default;
  constexpr Coordinate3D(float _x, float _y, float _z)
      : Coordinate2D(_x, _y), z(_z) {}
  bool operator==(const Coordinate3D& oth) {
    return Coordinate2D::operator==(oth) && z == oth.z;
  }
  bool operator!=(const Coordinate3D& oth) { return !(*this == oth); }
};

std::ostream& operator<<(std::ostream& os, const Coordinate2D& coord);
std::ostream& operator<<(std::ostream& os, const Coordinate3D& coord);

glm::vec3 RotateVec(glm::vec3 rotate_vec, glm::vec3 rotate_axis, float angle);
glm::mat4 GetDefaultProjection(int width, int height);

typedef Coordinate3D vec3;

void Normalize(Coordinate3D& vec);

class Mat4 {
 public:
  Mat4() : mat_(1.0f) {}
  Mat4& TranslateLeft(const vec3& pos_diff);
  Mat4& Translate(const vec3& pos_diff);
  Mat4& RotateLeft(float angle, const vec3& axis);
  Mat4& Rotate(float angle, const vec3& axis);
  Mat4& Scale(const vec3& vec);
  Mat4& Scale(float scale_size) {
    Scale({scale_size, scale_size, scale_size});
    return *this;
  }
  float* Ptr();
  Mat4& Reset() {
    mat_ = glm::mat4(1.f);
    return *this;
  }

 private:
  glm::mat4 mat_;
};
}  // namespace MatUtil
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_MAT_UTIL_H_
