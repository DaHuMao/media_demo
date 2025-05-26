#ifndef SDK_ANDROID_SRC_JNI_JNI_TOOL_JNI_MEMEBER_CONVERT_HELP_H_
#define SDK_ANDROID_SRC_JNI_JNI_TOOL_JNI_MEMEBER_CONVERT_HELP_H_
#include "sdk/android/src/jni/jni_tool//jni_help_tool.h"
namespace jni_help {
template <typename T>
int ConvertToNative(JNIEnv* jni, T& res,
                    typename jni_help::TypeToJniType<T>::jni_type java_obj) {
  jni_help::ContextType context;
  return jni_help::FunctionTrait<T>::get_native_member_func(jni, java_obj, res,
                                                            context);
}
template <typename T>
typename jni_help::TypeToJniType<T>::jni_type ConvertToJava(
    JNIEnv* jni, const T& value) {
  jni_help::ContextType context;
  return jni_help::FunctionTrait<T>::get_java_member_func(jni, value, context);
}
}  // namespace jni_help
#endif  // SDK_ANDROID_SRC_JNI_JNI_TOOL_JNI_MEMEBER_CONVERT_HELP_H_
