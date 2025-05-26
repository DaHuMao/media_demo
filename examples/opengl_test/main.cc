#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "shader.h"
#include "texture_load.h"
#include "render.h"
#include "camera.h"
#include "third_party/glm/glm.hpp"
#include "third_party/glm/gtc/matrix_transform.hpp"
#include "third_party/glm/gtc/type_ptr.hpp"

template <typename T, size_t N> char (&ArraySizeHelper(T (&array)[N]))[N];
#define ARRAY_SIZE(array) (sizeof(ArraySizeHelper(array)))

/*static void GlClearError() {
  while(glGetError() != GL_NO_ERROR) {};
  }

  static void GlCheckError() {
  while (GLenum error = glGetError()) {
  std::cout <<"[OpenGl Error]" << error <<std::endl;
  }
  }*/

constexpr int32_t kScreenWidth = 800;
constexpr int32_t kScreenHeight = 600;

float fov = 45;
bool first_mouse = true;
double last_x = kScreenWidth / 2;
double last_y = kScreenHeight / 2;
float delta_time = 0;
float last_time = 0;
Camera camera;

void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
  if (first_mouse) {
    last_x = xpos;
    last_y = ypos;
    first_mouse = false;
  }
  camera.ProcessMouseMovement(xpos - last_x, last_y - ypos);
  last_x = xpos;
  last_y = ypos;
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  fov -= yoffset;

  if (fov <= 1.0f) {
    fov = 1.0f;
  }

  if (fov >= 45.0f) {
    fov = 45.0f;
  }
}

void ProcessInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.PorcessKeyboard(CameraMovement::kForWard, delta_time);
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.PorcessKeyboard(CameraMovement::kBackWard, delta_time);
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.PorcessKeyboard(CameraMovement::kLeft, delta_time);
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.PorcessKeyboard(CameraMovement::kRight, delta_time);
  }
}



int main(void)
{
  GLFWwindow* window;
  /* Initialize the library */
  if (GLFW_TRUE != glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(kScreenWidth, kScreenHeight, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    if (GLEW_OK != glewInit()) {
        return -1;
    }
    {
        constexpr size_t kPointCount = 8;
        constexpr size_t kOneDimSize = 8;
        float positions[kPointCount * kOneDimSize] = {
            //     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
            0.5f,  0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // 右上
            0.5f, -0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // 右下
            -0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // 左下
            -0.5f,  0.5f, 0.5f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,    // 左上
            0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,   // 右上
            0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,   // 右下
            -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,   // 左下
            -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f    // 左上
        };

        glEnable(GL_DEPTH_TEST);
        VerTexBuffer vbo(positions, sizeof(positions));

        VerTexArray vao;
        VerTexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(2);
        vao.AddBuffer(layout);

        uint32_t indices[] = {
            // front
            0, 1, 2,
            2, 3, 0,

            // right
            4, 5, 1,
            1, 0, 4,

            // back
            4, 5, 6,
            6, 7, 4,

            // left
            6, 7, 3,
            3, 2, 6,

            // obove
            0, 4, 7,
            7, 3, 0,

            // under
            1, 5, 6,
            6, 2, 1
        };


        glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  -2.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };

        VerTexIndexBuffer ibo(indices, ARRAY_SIZE(indices));
        std::string vertexShader =
            "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "layout (location = 1) in vec3 aColor;\n"
            "layout (location = 2) in vec2 aTexCoord;\n"
            "out vec3 ourColor;"
            "out vec2 TexCoord;"
            "uniform mat4 model_mat;"
            "uniform mat4 view_mat;"
            "uniform mat4 projection_mat;"
            "void main()\n"
            "{\n"
                 "gl_Position = projection_mat * view_mat * model_mat * vec4(aPos, 1.0);\n"
                 "ourColor = aColor;\n"
                 "TexCoord = aTexCoord;\n"
            "}";
    /*    std::string fragmentShader =
            "#version 330 core\n"
            "uniform vec4 uColor;\n"
            "out vec4 FragColor;\n"
            "void main()\n"
            "{\n"
            "   FragColor = uColor;\n"
            "}\n";*/
        std::string fragmentShader =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec3 ourColor;\n"
            "in vec2 TexCoord;\n"
            "uniform vec4 uColor;\n"
            "uniform sampler2D ourTexture1;\n"
            "uniform sampler2D ourTexture2;\n"
            "void main()\n"
            "{\n"
            "    FragColor = uColor * mix(texture(ourTexture1, TexCoord),\
                            texture(ourTexture2, vec2(1 - TexCoord.x, TexCoord.y)), 0.6);\n"
            "}\n";
        Shader shader(vertexShader, fragmentShader);
        TextureLoad texture_load0("/Users/ztx/Desktop/workspace/opengl_test/res/container.jpg");
        TextureLoad texture_load1("/Users/ztx/Desktop/workspace/opengl_test/res/wawa.pic.jpg");
        shader.Bind();
        shader.SetUniform4f("uColor", 0.8f, 0.3f, 0.8f, 1.0f);
        shader.SetUniform1i("ourTexture1", 0);
        shader.SetUniform1i("ourTexture2", 1);

        vao.UnBind();
        vbo.UnBind();
        ibo.UnBind();
        glm::mat4 model_mat(1.0);
        glm::mat4 view_mat(1.0);
        glm::mat4 projection_mat(1.0);

        CustomRender render;
        float r = 0.0f, increment = 0.05;
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            auto now_time = glfwGetTime();
            delta_time = now_time - last_time;
            last_time = now_time;
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ProcessInput(window);
            if (r > 1.0) {
                increment = -0.05;
            } else if (r < 0.0) {
                increment = 0.05;
            }
            //glDrawArrays(GL_TRIANGLES, 0, 3);
            texture_load0.Bind(0);
            texture_load1.Bind(1);

            view_mat =  glm::mat4(1.0);
            view_mat = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0f, 0.0));
            view_mat = camera.GetViewMatrix();
           // view_mat = glm::translate(view_mat, glm::vec3(0.0f, 0.0f, -3.0f));
            projection_mat = glm::mat4(1.0);
            projection_mat = glm::perspective(glm::radians(fov),
                    1.0f * kScreenWidth / kScreenHeight,  0.1f, 100.0f);
            shader.SetUniform4f("uColor", r, 0.3f, 0.8f, 1.0f);
            shader.SetUniform4fv("view_mat", glm::value_ptr(view_mat));
            shader.SetUniform4fv("projection_mat", glm::value_ptr(projection_mat));
            for (size_t i = 0; i < ARRAY_SIZE(cubePositions); ++i) {
                model_mat = glm::mat4(1.0);
                model_mat = glm::translate(model_mat, cubePositions[i]);
                float angle =  20.0f * (i + 0.5);
                model_mat = glm::rotate(model_mat, static_cast<float>(now_time) *
                        glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                shader.SetUniform4fv("model_mat", glm::value_ptr(model_mat));
                render.Draw(vao, ibo, shader);

            }
            r += increment;
            /* Render here */
            /* Swap front and back buffers */
            glfwSwapBuffers(window);
            /* Poll for and process events */
            glfwPollEvents();
        }
    }
    glfwTerminate();
    return 0;
}
