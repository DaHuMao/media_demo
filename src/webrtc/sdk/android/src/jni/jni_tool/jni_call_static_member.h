#ifndef WEBRTC_SDK_ANDROID_SRC_JNI_JNI_TOOL_JNI_CALL_STATIC_MEMBER_H_
#define WEBRTC_SDK_ANDROID_SRC_JNI_JNI_TOOL_JNI_CALL_STATIC_MEMBER_H_
#include "rtc_base/checks.h"
#include "sdk/android/src/jni/jni_tool/jni_help_define.h"
namespace jni_help {
template<class T>
struct StaticFunctionTrait {
  static const std::function<void(JNIEnv*, jclass, jfieldID, T&)> get_java_static_member;
};

template<class T>
const std::function<void(JNIEnv*, jclass, jfieldID, T&)>
  StaticFunctionTrait<T>::get_java_static_member =
    [](JNIEnv* jni, jclass clazz, jfieldID id, T& out) -> void {
  ContextType context;
  auto obj = jni->GetStaticObjectField(clazz, id);
  jni_help::FunctionTrait<T>::get_native_member_func(
      jni, obj, out, context);
};

#define DefineStaticJNIBuiltInFunctionTrait(TypeName, Type) \
  template<> \
  struct StaticFunctionTrait<Type> { \
    static const std::function<void(JNIEnv*, jclass, jfieldID, Type&)> \
        get_java_static_member; \
  }; \

DefineStaticJNIBuiltInFunctionTrait(Int, int)
DefineStaticJNIBuiltInFunctionTrait(Long, long)
DefineStaticJNIBuiltInFunctionTrait(Float, float)
DefineStaticJNIBuiltInFunctionTrait(Boolean, bool)
DefineStaticJNIBuiltInFunctionTrait(Double, double)

template <class T>
int GetJavaBasicTypeStatic(JNIEnv* jni, jclass clazz,
                            const std::string& static_member_name,
                            T& return_val) {
  jfieldID id = jni->GetStaticFieldID(clazz, static_member_name.c_str(),
                                      TraitSignature<T>::signature_trait().c_str());
  if (0 == id) {
    RTC_DCHECK(false) << "GetStaticFieldID failed: " << static_member_name
                      << " signature: "
                      << TraitSignature<T>::signature_trait();
    return -1;
  }
  ContextType context;
  StaticFunctionTrait<T>::get_java_static_member(
      jni, clazz, id, return_val);
  return 0;
}
}  // namespace jni_help
#endif  // WEBRTC_SDK_ANDROID_SRC_JNI_JNI_TOOL_JNI_CALL_STATIC_MEMBER_H_
