#include "src/camera.h"
#include "src/const_data.h"
#include "src/gl_util.h"
#include "src/index_buffer.h"
#include "src/mat_util.h"
#include "src/render.h"
#include "src/shader.h"
#include "src/texture_load.h"
#include "src/vertex_array.h"
#include "src/vertex_buffer.h"
#include "src/windows.h"
#include <functional>
#include <iostream>
#include <string>

constexpr int32_t kScreenWidth = 1000;
constexpr int32_t kScreenHeight = 800;
int main(int argc, char *argv[]) {
  Windows &windows = Windows::Instance();
  windows.Init(kScreenWidth, kScreenHeight);
  {
    MatUtil::vec3 cubePositions[] = {
        {0.0f, 0.0f, 0.0f},     {2.0f, 5.0f, -15.0f}, {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f},
        {1.3f, -2.0f, -2.5f},   {1.5f, 2.0f, -2.5f},  {1.5f, 0.2f, -1.5f},
        {-1.3f, 1.0f, -1.5f}};
    std::string work_path = "../";
    std::string vertex_shader = work_path + "example/shader.vs";
    std::string fragment_shader = work_path + "example/shader.frag";
    TextureLoad texture_load0(work_path + "res/container.jpg");
    TextureLoad texture_load1(work_path + "res/awesomeface.jpg");
    VerTexBuffer vbo(kCubePositions, sizeof(kCubePositions));
    VerTexArray vao;
    VerTexBufferLayout layout;
    layout.Push<float>({3, 3, 2});
    vao.AddBuffer(layout);
    VerTexIndexBuffer ibo(kCubeIndices, ARRAY_SIZE(kCubeIndices));
    glm::mat4 projection =
        MatUtil::GetDefaultProjection(kScreenWidth, kScreenHeight);
    Shader shader(vertex_shader, fragment_shader);
    shader.Bind();
    shader.SetUniform4fv("projection", glm::value_ptr(projection));
    shader.SetUniform1i("selectTexture1", 0);
    shader.SetUniform1i("selectTexture2", 1);
    MatUtil::Mat4 model;
    CustomRender render;
    Camera camera;
    windows.BindCamera(&camera);
    windows.BindModelMat(&model);
    std::function<void()> func = [&]() {
      texture_load0.Bind(0);
      texture_load1.Bind(1);
      shader.SetUniform4fv("view", camera.GetViewMatrix());
      for (GLuint i = 0; i < 3; i++) {
        model.Reset().Translate(cubePositions[i]).Rotate(45, {0.5f, 0.5f, 0.f});
        shader.SetUniform4fv("model", model.Ptr());
        render.Draw(vao, ibo, shader);
      }
    };
    windows.join(func);
    vao.UnBind();
    vbo.UnBind();
    ibo.UnBind();
  }
  windows.Destroy();
  return 0;
}
