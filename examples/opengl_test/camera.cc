#include "camera.h"
#include "core/video_render/opengl/mat_util.h"
#include "third_party/glm/ext/matrix_float4x4.hpp"
#include "third_party/glm/ext/scalar_constants.hpp"
#include "third_party/glm/matrix.hpp"
namespace core {
Camera& Camera::RotateAroundX(float pitch) {
  if (pitch < -89) pitch = -89;
  if (pitch > 89) pitch = 89;
  auto camera_right = glm::cross(camera_up_, camera_front_);
  camera_front_ = MatUtil::RotateVec(camera_front_, camera_right, pitch);
  camera_up_ = MatUtil::RotateVec(camera_up_, camera_right, pitch);
  return *this;
}

Camera& Camera::RotateAroundY(float yaw) {
  camera_front_ = MatUtil::RotateVec(camera_front_, camera_up_, yaw);
  return *this;
}

Camera& Camera::RotateAroundZ(float roll) {
  camera_up_ = MatUtil::RotateVec(camera_up_, camera_front_, roll);
  return *this;
}
}  // namespace core

