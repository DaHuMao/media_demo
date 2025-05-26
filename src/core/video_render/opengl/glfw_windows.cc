#include "core/video_render/opengl/glfw_windows.h"
#include "GLFW/glfw3.h"
#include <ctime>
namespace core {
void GlfwWindows::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  auto win = static_cast<GlfwWindows*>(glfwGetWindowUserPointer(window));
  if (win && win->event_) {
    win->event_->OnMouseMoveEvent(xpos, ypos);
  }
}

void GlfwWindows::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  auto win = static_cast<GlfwWindows*>(glfwGetWindowUserPointer(window));
  if (win && win->event_) {
    win->event_->OnScrollEvent(xoffset, yoffset);
  }
}

// Is called whenever a key is pressed/released via GLFW
void GlfwWindows::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
  auto win = static_cast<GlfwWindows*>(glfwGetWindowUserPointer(window));
  if (win && win->event_) {
    win->event_->OnKeyEvent(key, scancode, action, mode);
  }
}

void GlfwWindows::windows_size_change_callback(GLFWwindow* window, int width,
    int height) {
  auto win = static_cast<GlfwWindows*>(glfwGetWindowUserPointer(window));
  if (win) {
    win->size_ = VideoSize(width, height);
  }
}

bool GlfwWindows::Bind() {
  if (!window_) {
    return false;
  }
  glfwMakeContextCurrent(window_);
  return true;
}

VideoSize GlfwWindows::GetSize() {
  return size_;
}

void GlfwWindows::SwapBuffers() {
  if (window_) {
    glfwSwapBuffers(window_);
  }
}
int GlfwWindows::Init(int width, int height, GlfwWindowsEvent* event) {
  /* Initialize the library */
  if (GLFW_TRUE != glfwInit())
    return -1;

  size_ = VideoSize(width, height);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  /* Create a window_ed mode window_ and its OpenGL context */
  window_ = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
  if (!window_)
  {
    glfwTerminate();
    return -1;
  }
  // GLFW Options
  //glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (!window_) {
    return -1;
  }
  glfwSetWindowUserPointer(window_, this);
  // Set the required callback functions
  glfwSetKeyCallback(window_, key_callback);
  glfwSetCursorPosCallback(window_, mouse_callback);
  glfwSetScrollCallback(window_, scroll_callback);
  glfwSetWindowSizeCallback(window_, windows_size_change_callback);
  event_ = event;
  return 0;
}
void GlfwWindows::Join(std::function<bool()> process) {
  while(!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    if (process) {
      if (!process()) {
        break;
      }
    }
  }
}
void GlfwWindows::Exit() {
  glfwSetWindowShouldClose(window_, GLFW_TRUE);
}

int GlfwWindows::Destroy() {
  if (window_) {
    glfwTerminate();
  }
  return 0;
}
}  // namespace core
