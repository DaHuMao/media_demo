#include "core/video_render/video_render_jni.h"

#include "sdk/android/src/jni/jni_tool/jni_call_static_member.h"
#include "sdk/android/src/jni/jni_tool/jni_help_define.h"
#include "sdk/android/src/jni/jni_tool/jni_help_tool.h"
#include "util/log.h"
namespace core {
constexpr char kTag[] = "VideoRenderJni";
VideoRenderJni::VideoRenderJni(jobject video_render_java_obj) {
  ::JavaVM* jvm = jni_help::GetJVM();
  /*jni_help::AttachThreadScoped ats(jvm);
  JNIEnv* jni = ats.env();
  auto factory_clazz = jni_help::FindClass(jni, "sdk/video/RenderFactory");
  int render_type = 0;
  jni_help::GetJavaBasicTypeStatic(jni, factory_clazz, "RENDER_TYPE_EGL",
                                   render_type);
  jobject video_render_java_obj = nullptr;
  int ret = jni_help::CallJavaStaticFunction(
      factory_clazz, "createRenderer", video_render_java_obj, render_type);
  RTC_DCHECK(ret == 0);*/
  jni_tool_ =
      std::make_unique<jni_help::NativeToJava>(jvm, video_render_java_obj);
  sequence_checker_.Detach();
  return;
}

VideoRenderJni::~VideoRenderJni() {
  jni_tool_.reset();
}
int VideoRenderJni::Init(core::Window* windows) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  int ret = 0;
  jni_tool_->CallFunction("init", ret);
  if (ret < 0) {
    LOGE_TAG(kTag) << "VideoRenderJni init failed";
    return -1;
  }
  is_init_ = true;
  return 0;
}

int VideoRenderJni::SendFrame(const core::VideoFrameDelegate& frame) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  if (!is_init_) {
    return -1;
  }
  RTC_DCHECK(is_playing_);
  if (window_size_ != window_->GetSize()) {
    window_size_ = window_->GetSize();
    window_->ReCreateSuface();
  }
  ReCreateRGBFrameIfNeed(frame.GetFormat().GetSize());
  video_convert_.Convert(frame, *video_frame_);

  int ret = 0;
  jni_tool_->CallFunction("sendFrame", ret, *video_frame_);
  return ret;
}

int VideoRenderJni::Play() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  is_playing_ = true;
  return 0;
}

int VideoRenderJni::Pause() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  is_playing_ = false;
  return 0;
}

int VideoRenderJni::UnInit() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  int ret = 0;
  jni_tool_->CallFunction("unInit", ret);
  is_init_ = false;
  return ret;
}

core::VideoSize VideoRenderJni::GetAlignSize(const core::VideoSize& size) {
  constexpr int alignment = 32;
  core::VideoSize size_alignment = size;
  size_alignment.width =
      size.width + (alignment - size.width % alignment) % alignment;
  return size_alignment;
}

void VideoRenderJni::ReCreateRGBFrameIfNeed(const core::VideoSize& size) {
  auto align_size = GetAlignSize(size);
  if (video_frame_ == nullptr ||
      video_frame_->GetFormat().GetSize() != align_size) {
    video_frame_ = std::make_unique<core::VideoFrame>(
        core::VideoFormatInfo(align_size, core::RawVideoFormat::kRGB));
  }
}
}  // namespace core
