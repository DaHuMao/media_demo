#include "core/video_render/video_render_impl.h"

#include "GL/glew.h"
#include "api/sequence_checker.h"
#include "core/video_render/opengl/gl_util.h"
#include "core/video_render/opengl/vertex_buffer_layout.h"
#include "rtc_base/arraysize.h"
#include "util/log.h"
namespace core {
// 顶点数据
constexpr float kVertices[] = {
    // 位置             // 纹理坐标
    1.0f,  1.0f,  0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f,  0.0f, 0.0f, 1.0f};
constexpr unsigned int kIndices[] = {0, 1, 3, 1, 2, 3};

constexpr char kShaderVertex[] =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";
constexpr char kShaderFragment[] =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D outTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(outTexture, vec2(TexCoord.x, 1- TexCoord.y));\n"
    "}\n";
constexpr char kTag[] = "VideoRenderImpl";
VideoRenderImpl::VideoRenderImpl() {
  sequence_checker_.Detach();
}

VideoRenderImpl::~VideoRenderImpl() { UnInit(); }
int VideoRenderImpl::Init(core::Window* window) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  if (!window) {
    LOGE_TAG(kTag) << "window is nullptr";
    return -1;
  }
  if (is_init_) {
    LOGW_TAG(kTag) << "VideoRenderImpl has been initialized";
    return -1;
  }
  window_ = window;
  LOGI_TAG(kTag) << "Init VideoRenderImpl"
                 << " windows size : " << window_->GetSize().ToString();
  window_size_ = window_->GetSize();
  InitGL();
  is_init_ = true;
  return 0;
}

int VideoRenderImpl::SendFrame(const core::VideoFrameDelegate& frame) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  if (!is_init_ || !is_playing_) {
    return -1;
  }
  ReCreateRGBFrameIfNeed(frame.GetFormat().GetSize());
  video_convert_.Convert(frame, *video_frame_);
  auto frame_view = core::RGBVideoFrameView(video_frame_->GetData()[0],
                                            video_frame_->GetFormat().GetSize(),
                                            core::RawVideoFormat::kRGB);
  RenderFrame(&frame_view);
  return error_code_;
}

int VideoRenderImpl::Play() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  is_playing_ = true;
  return 0;
}

int VideoRenderImpl::Pause() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  is_playing_ = false;
  return 0;
}

int VideoRenderImpl::UnInit() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  is_init_ = false;
  return 0;
}

bool VideoRenderImpl::InitGL() {
  if (is_init_) {
    return true;
  }
  if (!window_->Bind()) {
    LOGE_TAG(kTag) << "window bind failed";
    return false;
  }
  if (GLEW_OK != glewInit()) {
    RTC_DCHECK(false) << "glew init failed";
    return false;
  }
  // glEnable(GL_DEPTH_TEST);
  gl_info_ = std::make_unique<GLInfo>();
  gl_info_->shader_.Bind();
  gl_info_->shader_.SetUniform1i("outTexture", 0);
  GLint alignment;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
  return true;
}

void VideoRenderImpl::RenderFrame(core::RGBVideoFrameView* frame) {
  // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  static bool first = true;
  if (first) {
    first = false;
  }
  // glClear(GL_DEPTH_BUFFER_BIT);
  if (frame != nullptr) {
    gl_info_->texture_.Bind(*frame, 0);
  }
  gl_info_->vao_.Bind();
  RUN_FUNC_CHECK(glDrawElements(GL_TRIANGLES, gl_info_->ibo_.GetCount(),
                                GL_UNSIGNED_INT, nullptr));
  gl_info_->vao_.UnBind();
  window_->SwapBuffers();
}

core::VideoSize VideoRenderImpl::GetAlignSize(const core::VideoSize& size) {
  GLint alignment;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
  core::VideoSize size_alignment = size;
  size_alignment.width =
      size.width + (alignment - size.width % alignment) % alignment;
  return size_alignment;
}

void VideoRenderImpl::ReCreateRGBFrameIfNeed(const core::VideoSize& size) {
  auto align_size = GetAlignSize(size);
  bool need_recreate_vbo = false;
  if (video_frame_ == nullptr ||
      video_frame_->GetFormat().GetSize() != align_size) {
    video_frame_ = std::make_unique<core::VideoFrame>(
        core::VideoFormatInfo(align_size, core::RawVideoFormat::kRGB));
    need_recreate_vbo = true;
  }
    auto window_size = window_->GetSize();
    if (window_size != window_size_) {
      window_size_ = window_size;
      need_recreate_vbo = true;
      LOGI_TAG(kTag)
          << "window size changed: " << window_size.ToString();
    }
  if (need_recreate_vbo) {
    float vertices[sizeof(kVertices) / sizeof(float)];
    memcpy(vertices, kVertices, sizeof(kVertices));
    // width_new / win.height = size.width / size.height
    // x_diff = (win.width - width_new) / win.width / 2
    if (window_size.width * size.height > window_size.height * size.width) {
      float display_x_diff =
          (1.0 * window_size.width -
           1.0 * window_size.height * size.width / size.height) /
          window_size.width;
      vertices[0] -= display_x_diff;
      vertices[5] -= display_x_diff;
      vertices[10] += display_x_diff;
      vertices[15] += display_x_diff;
    } else {
      // height_new / win.width = size.height / size.width
      // y_diff = (win.height - height_new) / win.height / 2
      float display_y_diff =
          (1.0 * window_size.height -
           1.0 * window_size.width * size.height / size.width) /
          window_size.height;
      vertices[1] -= display_y_diff;
      vertices[6] += display_y_diff;
      vertices[11] += display_y_diff;
      vertices[16] -= display_y_diff;
    }
    gl_info_->ResetVboData(vertices, sizeof(vertices));
  }
}

VideoRenderImpl::GLInfo::GLInfo()
    : vbo_(kVertices, sizeof(kVertices)),
      ibo_(kIndices, arraysize(kIndices)),
      shader_(kShaderVertex, kShaderFragment, false) {
  core::VerTexBufferLayout layout;
  layout.Push<float>({3, 2});
  vao_.AddBuffer(layout);
  // ibo必须在vao之后绑定
  ibo_.UnBind();
  ibo_.Bind();
}

void VideoRenderImpl::GLInfo::ResetVboData(const void* data, size_t size) {
  vbo_.ResetData(data, size, true);
  core::VerTexBufferLayout layout;
  layout.Push<float>({3, 2});
  vao_.AddBuffer(layout);
  // ibo必须在vao之后绑定
  ibo_.UnBind();
  ibo_.Bind();
}

}  // namespace core
