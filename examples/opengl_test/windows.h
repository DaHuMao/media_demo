#ifndef OPENGL_TEST_WINDOWS_H_
#define OPENGL_TEST_WINDOWS_H_
#include <functional>

#include "camera.h"
#include "core/video_render/opengl/mat_util.h"
struct GLFWwindow;
namespace core {
class Windows {
 public:
  ~Windows() = default;
  int Init(int width, int height);
  bool join(std::function<void()> func);
  int Destroy();
  void BindCamera(Camera* camera);
  void BindModelMat(MatUtil::Mat4* model_mat);
  static Windows& Instance() {
    static Windows win;
    return win;
  }

 private:
  Windows() = default;
  GLFWwindow* window_ = nullptr;
};
}  // namespace core
#endif  // OPENGL_TEST_WINDOWS_H_
