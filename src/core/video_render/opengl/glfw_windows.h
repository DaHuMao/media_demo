#ifndef CORE_VIDEO_RENDER_OPENGL_GLFW_WINDOWS_H_
#define CORE_VIDEO_RENDER_OPENGL_GLFW_WINDOWS_H_
#include <functional>

#include "core/video_common/video_format_define.h"
#include "core/video_render//window.h"
struct GLFWwindow;
namespace core {
class GlfwWindowsEvent {
 public:
  virtual ~GlfwWindowsEvent() = default;
  virtual void OnKeyEvent(int key, int scancode, int action, int mode) = 0;
  virtual void OnMouseMoveEvent(double xpos, double ypos) = 0;
  virtual void OnScrollEvent(double xoffset, double yoffset) = 0;
};
class GlfwWindows final : public core::Window {
 public:
  GlfwWindows() = default;
  ~GlfwWindows() = default;
  bool Bind() override;
  VideoSize GetSize() override;
  void SwapBuffers() override;
  int ReCreateSuface() override { return 0; }
  int Init(int width, int height, GlfwWindowsEvent* event);
  void Join(std::function<bool()> process);
  void Exit();
  int Destroy();

 private:
  static void key_callback(GLFWwindow* window, int key, int scancode,
                           int action, int mode);
  static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
  static void scroll_callback(GLFWwindow* window, double xoffset,
                              double yoffset);
  static void windows_size_change_callback(GLFWwindow* window, int width,
      int height);
  GLFWwindow* window_ = nullptr;
  GlfwWindowsEvent* event_ = nullptr;
  VideoSize size_;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_OPENGL_GLFW_WINDOWS_H_
