#ifndef CORE_VIDEO_RENDER_VIDEO_RENDER_JNI_H_
#define CORE_VIDEO_RENDER_VIDEO_RENDER_JNI_H_
#include <jni.h>
#include <memory>
#include "api/sequence_checker.h"
#include "core/video_common/video_convert.h"
#include "core/video_common/video_frame.h"
#include "core/video_render/video_render.h"
namespace jni_help {
class NativeToJava;
} // namespace jni_help
namespace core {
class VideoRenderJni : public VideoRender {
 public:
  VideoRenderJni(jobject video_render_java_obj);
  ~VideoRenderJni() override;
  int Init(core::Window* windows) override;
  int SendFrame(const core::VideoFrameDelegate& frame) override;
  int Play() override;
  int Pause() override;
  int UnInit() override;

 private:
  core::VideoSize GetAlignSize(const core::VideoSize& size);
  void ReCreateRGBFrameIfNeed(const core::VideoSize& size);
 private:
  bool is_playing_ = false;
  bool is_init_ = false;
  core::Window* window_ = nullptr;
  VideoSize window_size_;
  core::VideoConvert video_convert_;
  std::unique_ptr<jni_help::NativeToJava> jni_tool_;
  webrtc::SequenceChecker sequence_checker_;
  std::unique_ptr<core::VideoFrame> video_frame_;
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_VIDEO_RENDER_JNI_H_
