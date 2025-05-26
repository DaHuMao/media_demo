#ifndef CORE_VIDEO_RENDER_OPENGL_WINDOWS_H_
#define CORE_VIDEO_RENDER_OPENGL_WINDOWS_H_
#include <functional>

#include "core/video_render/opengl/mat_util.h"
struct GLFWwindow;
namespace core {
class GlfwWindowsEvent {
 public:
  virtual ~GlfwWindowsEvent() = default;
  virtual void OnKeyEvent(int key, int scancode, int action, int mode) = 0;
  virtual void OnMouseMoveEvent(double xpos, double ypos) = 0;
  virtual void OnScrollEvent(double xoffset, double yoffset) = 0;
};
class GlfwWindows {
 public:
  GlfwWindows() = default;
  ~GlfwWindows() = default;
  int Init(int width, int height, GlfwWindowsEvent* event);
  void Exit();
  void Join();
  int Destroy();
  GLFWwindow* GetWindow() { return window_; }

 private:
  static void key_callback(GLFWwindow* window, int key, int scancode,
                           int action, int mode);
  static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
  static void scroll_callback(GLFWwindow* window, double xoffset,
                              double yoffset);
  GLFWwindow* window_ = nullptr;
  GlfwWindowsEvent* event_ = nullptr;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_WINDOWS_H_
