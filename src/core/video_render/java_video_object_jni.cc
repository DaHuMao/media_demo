
#include <jni.h>
#include "core/video_common/video_format_define.h"
#include "core/video_common/video_format_info.h"
#include "core/video_common/video_frame.h"
#include "sdk/android/src/jni/jni_tool/jni_help_define.h"
#include "util/array_find.h"
CUSTOMER_DEFINE_JNI_SIGANTURE(core::VideoSize, "sdk/video/VideoSize")
CUSTOMER_DEFINE_JNI_SIGANTURE(core::VideoFormatInfo,
                              "sdk/video/VideoFormatInfo")
CUSTOMER_DEFINE_JNI_SIGANTURE(core::VideoFrame, "sdk/video/VideoFrame")

jobject GetJavaVideoSize(JNIEnv* jni, const core::VideoSize& size,
                         jni_help::ContextTypeRef context) {
  jni_help::JavaObjectMaker obj(
      jni,
      jni_help::ClassBaseSignature<core::VideoSize>::signature_trait().c_str(),
      context, size.width, size.height);
  return obj.get_object();
}

static constexpr std::pair<core::RawVideoFormat, int> kFormatMap[] = {
    {core::RawVideoFormat::kYUV420P, 0}, {core::RawVideoFormat::kNV12, 1},
    {core::RawVideoFormat::kRGBA, 2},    {core::RawVideoFormat::kBGRA, 3},
    {core::RawVideoFormat::kRGB, 4},     {core::RawVideoFormat::kBGR, 5},
};

jobject GetJavaVideoFormatInfo(JNIEnv* jni,
                               const core::VideoFormatInfo& format_info,
                               jni_help::ContextTypeRef context) {
  jni_help::JavaObjectMaker obj(
      jni,
      jni_help::ClassBaseSignature<core::VideoFormatInfo>::signature_trait()
          .c_str(),
      context, GetJavaVideoSize(jni, format_info.GetSize(), context),
      util::ArrayFind(kFormatMap, format_info.GetFormat(), -1),
      format_info.GetStrides());
  return obj.get_object();
}

jobject GetJavaVideoFrame(JNIEnv* jni,
                               const core::VideoFrame& frame,
                               jni_help::ContextTypeRef context) {
  jni_help::JavaObjectMaker obj(
      jni,
      jni_help::ClassBaseSignature<core::VideoFrame>::signature_trait()
          .c_str(),
      context, GetJavaVideoFormatInfo(jni, frame.GetFormat(), context),
      jni->NewDirectByteBuffer(
        reinterpret_cast<void*>(frame.GetOriginalPoint()),
        frame.GetFormat().GetAllSizeInByte()));
  return obj.get_object();
}

CUSTOMER_DEFINE_JNI_JAVA_FUNCTION_TRAIT(core::VideoSize, GetJavaVideoSize)
CUSTOMER_DEFINE_JNI_JAVA_FUNCTION_TRAIT(core::VideoFormatInfo,
                                        GetJavaVideoFormatInfo)
CUSTOMER_DEFINE_JNI_JAVA_FUNCTION_TRAIT(core::VideoFrame, GetJavaVideoFrame)
