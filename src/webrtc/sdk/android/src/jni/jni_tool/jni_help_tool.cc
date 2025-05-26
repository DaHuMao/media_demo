#include "jni_help_tool.h"
#include "jni_help_define.h"
#include "rtc_base/checks.h"
#include "src/util/log.h"

namespace jni_help {
static ::JavaVM *g_jvm_instance = nullptr;
static jobject g_class_loader = nullptr;
static jmethodID g_find_class_method_id = nullptr;

::JavaVM *GetJVM() { return g_jvm_instance; }
::JNIEnv *GetEnv() { return AttachThreadScoped(g_jvm_instance).env(); }

void InitJVM(::JavaVM *jvm, jobject class_loader) {
  if (nullptr != g_jvm_instance) {
    return;
  }
  g_jvm_instance = jvm;
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  auto loadClass = env->FindClass("java/lang/ClassLoader");
  g_find_class_method_id =  env->GetMethodID(
      loadClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
  RTC_DCHECK(g_find_class_method_id != 0);
  g_class_loader = env->NewGlobalRef(class_loader);
}

void ClearException(JNIEnv* jni) {
  if (jni->ExceptionCheck()) {
    jni->ExceptionDescribe(); // 输出异常日志
    jni->ExceptionClear();   // 必须清除异常，否则下次 JNI 调用会崩溃
  }
}

jclass FindClass(JNIEnv *jni, const char *name) {
  RTC_DCHECK(name != nullptr);
  RTC_DCHECK(jni != nullptr);
  jclass clazz = jni->FindClass(name);
  if (clazz == nullptr){
    ClearException(jni);
    RTC_DCHECK(g_class_loader != nullptr) << "class loader is null";
    jstring j_name = jni->NewStringUTF(name);
    clazz = reinterpret_cast<jclass>(
        jni->CallObjectMethod(g_class_loader, g_find_class_method_id, j_name));
    jni->DeleteLocalRef(j_name);
  }
  RTC_CHECK(clazz != nullptr) << "FindClass failed: " << name;
  return clazz;
}

JavaToNative::JavaToNative(JNIEnv *jni, jobject obj) : _jni(jni) {
  if (nullptr != _jni && nullptr != obj) {
    jclass local_clazz = _jni->GetObjectClass(obj);
    _clazz = reinterpret_cast<jclass>(jni->NewGlobalRef(local_clazz));
    _obj = reinterpret_cast<jobject>(jni->NewGlobalRef(obj));
    jni->DeleteLocalRef(local_clazz);
  }
}

JavaToNative::~JavaToNative() {
  if (nullptr != _clazz) {
    _jni->DeleteGlobalRef(_clazz);
  }
  if (nullptr != _obj) {
    _jni->DeleteGlobalRef(_obj);
  }
}

std::unique_ptr<JavaToNative> JavaToNative::GetObjectMember(const char *name,
                                                            const char *type) {
  if (nullptr == _jni || nullptr == _obj) {
    return nullptr;
  }
  jfieldID id = _jni->GetFieldID(_clazz, name, type);
  jobject obj = _jni->GetObjectField(_obj, id);
  return std::unique_ptr<JavaToNative>(new JavaToNative(_jni, obj));
}

NativeToJava::NativeToJava(JavaVM *jvm, jobject obj) : _jvm(jvm) {
  if (nullptr != _jvm && nullptr != obj) {
    AttachThreadScoped ats(_jvm);
    JNIEnv *jni = ats.env();
    _obj = reinterpret_cast<jobject>(jni->NewGlobalRef(obj));
    jclass local_clazz = jni->GetObjectClass(_obj);
    _clazz = reinterpret_cast<jclass>(jni->NewGlobalRef(local_clazz));
    jni->DeleteLocalRef(local_clazz);
  }
}

NativeToJava::~NativeToJava() {
  AttachThreadScoped ats(_jvm);
  JNIEnv *jni = ats.env();
  if (nullptr != _clazz) {
    jni->DeleteGlobalRef(_clazz);
  }
  if (nullptr != _obj) {
    jni->DeleteGlobalRef(_obj);
  }
}

int NativeToJava::CallVoidFunction(const char *function_name) {
  if (nullptr == _jvm || nullptr == _clazz || nullptr == _obj) {
    return -1;
  }
  AttachThreadScoped ats(_jvm);
  JNIEnv *jni = ats.env();
  std::string signature = "()V";
  jmethodID id = jni->GetMethodID(_clazz, function_name, signature.c_str());
  if (0 == id) {
    return -1;
  }
  jni->CallVoidMethod(_obj, id);
  return 0;
}

int CallJavaStaticVoidFunction(jclass clazz, const char *function_name) {
  RTC_DCHECK(clazz != nullptr);
  AttachThreadScoped ats(jni_help::GetJVM());
  JNIEnv *jni = ats.env();
  std::string signature = "()V";
  jmethodID id =
      jni->GetStaticMethodID(clazz, function_name, signature.c_str());
  if (0 == id) {
    RTC_DCHECK(false);
    return -1;
  }
  ContextType context;
  jni->CallStaticVoidMethod(clazz, id);
  return 0;
}

jobject GetJavaInstance(const std::string &class_signature,
                        const std::string &get_instance_func_name) {
  return GetJavaObjectStatic(
        FindClass(GetEnv(), class_signature.c_str()),
      get_instance_func_name, class_signature);
}

jobject GetJavaInstance(JNIEnv* jni,
    const std::string& class_signature,
    const std::string& get_instance_func_name,
    const std::string& java_obj_signature) {
  return GetJavaObjectStatic(
      FindClass(jni, class_signature.c_str()), get_instance_func_name,
      java_obj_signature);
}

jobject GetJavaObjectStatic(jclass clazz, const std::string &func_name,
                            const std::string& java_obj_signature) {
  RTC_DCHECK(clazz != nullptr);
  AttachThreadScoped ats(jni_help::GetJVM());
  JNIEnv *jni = ats.env();
  std::string signature = "()L" + java_obj_signature + ";";
  jmethodID id = jni->GetStaticMethodID(clazz, func_name.c_str(),
                                        signature.c_str());
  if (0 == id) {
    RTC_CHECK(false) << "GetStaticMethodID failed: " << func_name
      << " signature: " << signature;
    return nullptr;
  }
  return CallStaticObjectFunc(jni, clazz, id);
}

void CallJavaFunctionWithObj(
    JNIEnv* jni, jobject obj, const char* function_name) {
  RTC_DCHECK(jni != nullptr);
  RTC_DCHECK(obj != nullptr);
  jclass clazz = jni->GetObjectClass(obj);
  RTC_DCHECK(clazz != nullptr);
  jmethodID id = jni->GetMethodID(clazz, function_name, "()V");
  RTC_DCHECK(id != 0);
  jni->CallVoidMethod(obj, id);
  jni->DeleteLocalRef(clazz);
}

}  // namespace jni_help
