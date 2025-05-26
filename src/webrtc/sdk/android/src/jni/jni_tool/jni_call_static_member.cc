#include "sdk/android/src/jni/jni_tool/jni_call_static_member.h"
namespace jni_help {
#define ImplementStaticJNIBuiltInFunctionTrait(TypeName, Type) \
    const std::function<void(JNIEnv*, jclass, jfieldID, Type&)> \
        StaticFunctionTrait<Type>::get_java_static_member = \
        [](JNIEnv* jni, jclass clazz, jfieldID id, Type& out) -> void { \
          out = jni->GetStatic##TypeName##Field(clazz, id); \
        }; \

ImplementStaticJNIBuiltInFunctionTrait(Int, int)
ImplementStaticJNIBuiltInFunctionTrait(Long, long)
ImplementStaticJNIBuiltInFunctionTrait(Float, float)
ImplementStaticJNIBuiltInFunctionTrait(Boolean, bool)
ImplementStaticJNIBuiltInFunctionTrait(Double, double)
}  // namespace jni_help
