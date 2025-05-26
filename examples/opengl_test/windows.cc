#include "windows.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <ctime>
#include "camera.h"
#include "core/video_render/opengl/gl_util.h"
#include "core/video_render/opengl/mat_util.h"

namespace core {
static bool keys[1024];
int32_t screenWidth = 1000;
int32_t screenHeight = 800;

Camera* sg_camera = nullptr;
MatUtil::Mat4* sg_model = nullptr;
MatUtil::vec3 sg_model_translate;
bool sg_use_camera = false;
GLfloat yaw =
    -90.0f; // Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in
            // a direction vector pointing to the right (due to how Eular angles
            // work) so we initially rotate a bit to the left.
GLfloat pitch = 0.0f;
GLfloat lastX =  screenHeight / 2.0;
GLfloat lastY = screenHeight / 2.0;
GLfloat fov = 45.0f;

// Deltatime
GLfloat deltaTime = 0.0f; // Time between current frame and last frame
GLfloat lastFrame = 0.0f; // Time of last frame
void RotateAroundX(float angle_diff) {
  if (sg_use_camera) {
    sg_camera->RotateAroundX(40 * angle_diff);
  } else {
    sg_model->RotateLeft(angle_diff, {1.0f, 0, 0});
  }
}

void RotateAroundY(float angle_diff) {
  if (sg_use_camera) {
    sg_camera->RotateAroundY(40 * angle_diff);
  } else {
    sg_model->RotateLeft(angle_diff, {0.0f, 1.0f, 0});
  }
}

void RotateAroundZ(float angle_diff) {
  if (sg_use_camera) {
    sg_camera->RotateAroundZ(40 * angle_diff);
  } else {
    sg_model->RotateLeft(angle_diff, {0.0f, 0.0f, 1.0f});
  }
}

void MoveX(float diff) {
  if (sg_use_camera) {
    sg_camera->MoveCenterX(diff);
  } else {
    sg_model_translate.x = diff;
  }
}

void MoveY(float diff) {
  if (sg_use_camera) {
    sg_camera->MoveCenterY(diff);
  } else {
    sg_model_translate.y = diff;
  }
}

void MoveZ(float diff) {
  if (sg_use_camera) {
    sg_camera->ScaleCameraDistance(diff);
  } else {
    sg_model_translate.z = diff;
  }
}

void do_movement() {
  // Camera controls
  GLfloat cameraSpeed = 2.0f * deltaTime;
  GLfloat angle_diff = cameraSpeed;
  GLfloat camera_center_diff = cameraSpeed;
  sg_model_translate = {0, 0, 0};
  if (keys[GLFW_KEY_1]) {
    sg_use_camera = true;
  }
  if (keys[GLFW_KEY_2]) {
    sg_use_camera = false;
  }
  if (keys[GLFW_KEY_Q])
    MoveZ(cameraSpeed);
  if (keys[GLFW_KEY_E])
    MoveZ(-cameraSpeed);
  if (keys[GLFW_KEY_W])
    RotateAroundX(angle_diff);
  if (keys[GLFW_KEY_S])
    RotateAroundX(-angle_diff);
  if (keys[GLFW_KEY_A])
    RotateAroundY(angle_diff);
  if (keys[GLFW_KEY_D])
    RotateAroundY(-angle_diff);
  if (keys[GLFW_KEY_T])
    RotateAroundZ(angle_diff);
  if (keys[GLFW_KEY_Y])
    RotateAroundZ(-angle_diff);
  if (keys[GLFW_KEY_RIGHT])
    MoveX(-camera_center_diff);
  if (keys[GLFW_KEY_LEFT])
    MoveX(camera_center_diff);
  if (keys[GLFW_KEY_UP])
    MoveY(-camera_center_diff);
  if (keys[GLFW_KEY_DOWN])
    MoveY(camera_center_diff);

  if (sg_model_translate != MatUtil::Coordinate3D()) {
    sg_model->TranslateLeft(sg_model_translate);
  }
}

bool firstMouse = true;
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  GLfloat xoffset = xpos - lastX;
  GLfloat yoffset =
      lastY - ypos; // Reversed since y-coordinates go from bottom to left
  lastX = xpos;
  lastY = ypos;

  GLfloat sensitivity = 0.05; // Change this value to your liking
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // Make sure that when pitch is out of bounds, screen doesn't get flipped
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  if (fov >= 1.0f && fov <= 45.0f)
    fov -= yoffset;
  if (fov <= 1.0f)
    fov = 1.0f;
  if (fov >= 45.0f)
    fov = 45.0f;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

int Windows::Init(int width, int height) {
  /* Initialize the library */
  if (GLFW_TRUE != glfwInit())
    return -1;

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
  screenWidth = width;
  screenHeight = height;
  glfwMakeContextCurrent(window_);
  // GLFW Options
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (GLEW_OK != glewInit()) {
    return -1;
  }
  // Set the required callback functions
  glfwSetKeyCallback(window_, key_callback);
  glfwSetCursorPosCallback(window_, mouse_callback);
  glfwSetScrollCallback(window_, scroll_callback);
  glEnable(GL_DEPTH_TEST);
  return 0;
}

bool Windows::join(std::function<void()> func) {
  if (!window_) {
    return false;
  }
  while (!glfwWindowShouldClose(window_)) {
      glfwPollEvents();
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      if (GlCheckError() || GlHasError()) {
        break;
      }
      if (func) {
        func();
      }
      glfwSwapBuffers(window_);
      do_movement();
      auto current_time = glfwGetTime();
      deltaTime = current_time - lastFrame;
      lastFrame = current_time;
  }
  return 0;
}

int Windows::Destroy() {
  if (window_) {
    glfwTerminate();
  }
  return 0;
}

void Windows::BindCamera(Camera* camera) {
  sg_camera = camera;
}

void Windows::BindModelMat(MatUtil::Mat4* model_mat) {
  sg_model = model_mat;
}
}  // namespace core
