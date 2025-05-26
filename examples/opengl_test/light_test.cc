#include <functional>
#include <string>

#include "camera.h"
#include "const_data.h"
#include "core/video_common/video_frame.h"
#include "core/video_render/opengl/gl_texture.h"
#include "core/video_render/opengl/gl_util.h"
#include "core/video_render/opengl/mat_util.h"
#include "core/video_render/opengl/shader.h"
#include "core/video_render/opengl/vertex_array.h"
#include "core/video_render/opengl/vertex_buffer.h"
#include "core/video_render/opengl/vertex_index_buffer.h"
#include "examples/opengl_test/picture_loader.h"
#include "render.h"
#include "windows.h"

constexpr int32_t kScreenWidth = 1000;
constexpr int32_t kScreenHeight = 800;

struct Position {
  core::MatUtil::Coordinate3D pos;
  core::MatUtil::Coordinate3D normal;
  core::MatUtil::Coordinate2D tex;
};

int main() {
  core::Windows& windows = core::Windows::Instance();
  windows.Init(kScreenWidth, kScreenHeight);
  {
    std::string work_path =
        "/Users/ztx/Desktop/workspace/media_demo/examples/opengl_test/";
    std::string lamp_vertex_shader = work_path + "shader/lamp_shader.vs";
    std::string lamp_fragment_shader = work_path + "shader/lamp_shader.frag";
    // constexpr size_t kCubeSize = kPointCount * (3 + 3);
    Position cube[kPointCount];
    for (size_t i = 0; i < kPointCount; ++i) {
      auto coord = kCubeCoordinate[i];
      auto& position = cube[i];
      position.pos = coord;
      position.normal.x = coord.x > 0 ? 1.0f : -1.0f;
      position.normal.y = coord.y > 0 ? 1.0f : -1.0f;
      position.normal.z = coord.z > 0 ? 1.0f : -1.0f;
      core::MatUtil::Normalize(position.normal);
      position.tex.x = kCubePositions[kOneDimSize * i + 6];
      position.tex.y = kCubePositions[kOneDimSize * i + 7];
    }
    core::MatUtil::Coordinate3D lampPos = {1.2, 1.0, -2};
    std::string vertex_shader = work_path + "shader/light_shader.vs";
    std::string fragment_shader = work_path + "shader/light_shader.frag";
    core::GlTexture texture_load0;
    core::GlTexture texture_load1;
    PictureLoader picture_loader0(work_path + "res/container2.png");
    PictureLoader picture_loader1(work_path + "res/container2_specular.png");
    core::VerTexBuffer vbo(cube, sizeof(cube));
    core::VerTexArray vao;
    core::VerTexBufferLayout layout;
    layout.Push<float>({3, 3, 2});
    vao.AddBuffer(layout);
    core::VerTexIndexBuffer ibo(kCubeIndices, ARRAY_SIZE(kCubeIndices));
    glm::mat4 projection =
        core::MatUtil::GetDefaultProjection(kScreenWidth, kScreenHeight);
    core::Shader light_shader(vertex_shader, fragment_shader);
    light_shader.Bind();
    light_shader.SetUniform4fv("projection", glm::value_ptr(projection));
    light_shader.SetUniform1i("material.diffuse", 0);
    light_shader.SetUniform1i("material.specular", 1);
    light_shader.SetUniform1f("material.shininess", 32.f);
    light_shader.SetUniform3f("light.position", lampPos);
    light_shader.SetUniform3f("light.diffuse", {0.5f, 0.5f, 0.5f});
    light_shader.SetUniform3f("light.specular", {1.f, 1.f, 1.f});
    light_shader.SetUniform3f("light.ambient", {0.2f, 0.2f, 0.2f});

    light_shader.SetUniform3f("lampPos", lampPos);
    core::Shader lamp_shader(vertex_shader, lamp_fragment_shader);
    lamp_shader.Bind();
    lamp_shader.SetUniform4fv("projection", glm::value_ptr(projection));
    core::MatUtil::Mat4 model_light;
    core::MatUtil::Mat4 model_lamp;
    model_lamp.Translate(lampPos).Scale(0.1f);
    CustomRender render;
    core::Camera camera;
    windows.BindCamera(&camera);
    windows.BindModelMat(&model_light);
    core::RGBVideoFrameView frame1(picture_loader0.GetData(),
                                   picture_loader0.GetSize(),
                                   core::RawVideoFormat::kRGB);
    core::RGBVideoFrameView frame2(picture_loader1.GetData(),
                                   picture_loader1.GetSize(),
                                   core::RawVideoFormat::kRGB);
    std::function<void()> func = [&]() {
      texture_load0.Bind(frame1, 0);
      texture_load1.Bind(frame2, 1);
      light_shader.Bind();
      light_shader.SetUniform4fv("view", camera.GetViewMatrix());
      light_shader.SetUniform4fv("model", model_light.Ptr());
      light_shader.SetUniform3f("viewPos", camera.GetViewPos());
      render.Draw(vao, ibo, light_shader);

      lamp_shader.Bind();
      lamp_shader.SetUniform4fv("view", camera.GetViewMatrix());
      lamp_shader.SetUniform4fv("model", model_lamp.Ptr());
      render.Draw(vao, ibo, lamp_shader);
    };
    windows.join(func);
    vao.UnBind();
    vbo.UnBind();
    ibo.UnBind();
  }
  windows.Destroy();
  return 0;
}
