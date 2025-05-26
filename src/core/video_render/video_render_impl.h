#ifndef CORE_VIDEO_RENDER_IMPL_H_
#define CORE_VIDEO_RENDER_IMPL_H_
#include "api/sequence_checker.h"
#include "core/video_common/video_convert.h"
#include "core/video_common/video_format_define.h"
#include "core/video_common/video_frame.h"
#include "core/video_render//window.h"
#include "core/video_render/opengl/gl_texture.h"
#include "core/video_render/opengl/shader.h"
#include "core/video_render/opengl/vertex_array.h"
#include "core/video_render/opengl/vertex_buffer.h"
#include "core/video_render/opengl/vertex_index_buffer.h"
#include "core/video_render/video_render.h"
namespace core {
class VideoRenderImpl : public VideoRender {
 public:
  VideoRenderImpl();
  ~VideoRenderImpl() override;
  int Init(core::Window* window) override;
  int SendFrame(const core::VideoFrameDelegate& frame) override;
  int Play() override;
  int Pause() override;
  int UnInit() override;

 private:
  struct GLInfo {
    GLInfo();
    void ResetVboData(const void* data, size_t size);
    core::VerTexBuffer vbo_;
    core::VerTexArray vao_;
    core::VerTexIndexBuffer ibo_;
    core::Shader shader_;
    core::GlTexture texture_;
  };

  bool InitGL();
  void RenderFrame(core::RGBVideoFrameView* frame);
  core::VideoSize GetAlignSize(const core::VideoSize& size);
  void ReCreateRGBFrameIfNeed(const core::VideoSize& size);
  bool is_init_ = false;
  bool is_playing_ = false;
  int error_code_ = 0;
  core::VideoSize window_size_;
  core::VideoConvert video_convert_;
  std::unique_ptr<core::VideoFrame> video_frame_;
  std::unique_ptr<GLInfo> gl_info_;
  core::Window* window_ = nullptr;
  webrtc::SequenceChecker sequence_checker_;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_IMPL_H_
