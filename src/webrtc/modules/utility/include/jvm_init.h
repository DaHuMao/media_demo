#ifndef MODULES_UTILITY_INCLUDE_JVM_INIT_H_
#define MODULES_UTILITY_INCLUDE_JVM_INIT_H_
#include <jni.h>
namespace webrtc {
namespace jvm_helper {
  void InitJVM(JavaVM* vm);
  JavaVM* GetJVM();
} // namespace jvm_helper
}  // namespace webrtc
#endif  // MODULES_UTILITY_INCLUDE_JVM_INIT_H_
