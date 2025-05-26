#ifndef JNI_COMMON_JNI_HELP_TOOL_H_
#define JNI_COMMON_JNI_HELP_TOOL_H_

#include "modules/utility/include/helpers_android.h"
#include "jni_help_define.h"

namespace jni_help {
template <class T>
struct JniPair {
  JniPair(std::string&& that_key, T& that_val)
      : key(that_key), value(that_val) {}
  std::string key;
  T& value;
};

::JavaVM* GetJVM();
::JNIEnv* GetEnv();
void InitJVM(::JavaVM* jvm, jobject class_loader);
jclass FindClass(JNIEnv* jni, const char* name);
void ClearException(JNIEnv* jni);

using AttachThreadScoped = webrtc::AttachThreadScoped;

class JavaToNative final {
 public:
  JavaToNative(JNIEnv* jni, jobject obj);
  ~JavaToNative();

  /*
   * EXAMPLE:
   *      Java define:
   *      package A.B.C
   *      class TestJava {
   *          public static TestJavaSub {
   *              String mName;
   *          }
   *          private int mIndex;
   *          private float mA;
   *          private double mB;
   *          private int[][][] mIntArr;
   *          private short[] mShortArr;
   *          private TestJavaSub mTestSub;
   *      }
   *      C define:
   *      struct TestC++Sub {
   *          const char* name;
   *      };
   *      struct TestC++ {
   *          int index;
   *          float a;
   *          double b;
   *          std::vector<std::vector<std::vector<int>>> int_arr;
   *          int16_t* short_arr;
   *          int short_arr_size;
   *          TestC++Sub test_sub;
   *      };
   *=================================== ===================================
   *You can parse it as follows:
   *int parsing_TestC(JNIEnv* jni, jclass clazz, jobject obj, TestC& out) {
   *  jni_help::JavaToNative jni_tool(jni, obj);
   *
   *  const char* member_name = {"mIndex", "mA", "mB", "mIntArr", "mShortArr"};
   *  if (jni_tool.GetMember(nullptr, member_name, ARRAY_SIZE(member_name),
   *                         out.index, out.a, out.b, out.mShortArr,
   *                         jni_help::CStyleArrayRef<int16_t>(
   *                             out.short_arr, out.short_arr_size)) < 0) {
   *    return -1;
   *  }
   *
   *  std::unique_ptr<JavaArrayToNative> jni_tool_sub =
   *      jni_tool.GetObjectMember("mTestSub", "LA/B/C/TestJava$TestJavaSub");
   *  if (jni_tool_sub->GetMember(nullptr,
   *            JniPair<jni_help::JavaArrayToNative<char>>("mName",
   *              out.test_sub.name) < 0) {
   *    return -1;
   *  }
   *  return 0;
   *}
   *=================================== ===================================
   * A more advanced use is as follows：
   * template<class T>
   * jni_help::JniPair<T> jni_pair(std::string&& that_key, T& that_val) {
   *    return jni_help::JniPair<T>(std::forward<std::string>(that_key),
   *                                that_val);
   * }
   *
   * int parsing_TestCSub(JNIEnv*, jobject, TestC++Sub&, ContextTypeRef);
   *
   * CUSTOMER_DEFINE_JNI_SIGANTURE(TestC++Sub, "LA/B/C/TestJava$TestJavaSub")
   * CUSTOMER_DEFINE_JNI_NATIVE_FUNCTION_TRAIT(TestC++Sub, parsing_TestCSub)
   *
   * int parsing_TestCSub(JNIEnv* jni, jobject obj, TestC++Sub& out,
   *     jni_help::ContextTypeRef context) {
   *     jni_help::JavaToNative jni_tool(jni, obj);
   *     if (jni_tool->GetMember(context,
   *          jni_pair("mName", out.name)) < 0) {
   *           return -1;
   *      }
   *      return 0;
   * }
   *
   * int parsing_TestC(JNIEnv* jni, jobject obj, TestC& out) {
   *     jni_help::JavaToNative jni_tool(jni, obj);
   *     if (jni_tool->GetMember(reinterpret_cast<ContextTypeRef>(&context),
   *                             jni_pair("mIndex", out.index),
   *                             jni_pair("mA", out.a),
   *                             jni_pair("mB", out.b),
   *                             jni_pair("mIntArr", out.int_arr),
   *                             jni_pair("mShortArr",
   *                                  CStyleArrayRef<int16_t>(out.short_arr,
   *                                  out.short_arr_size)),
   *                             jni_pair("mTestSub", out.test_sub)) < 0) {
   *       return -1;
   *     }
   *     return 0;
   * }
   *
   *=================================== ===================================
   *
   */
  int GetMember(const char** name, size_t name_size, ContextTypeRef context) {
    return 0;
  }
  int GetMember(ContextTypeRef context) { return 0; }
  template <class T, class... Args>
  int GetMember(ContextTypeRef context, const char** name, size_t name_size,
                T& cur, Args&... args);

  template <class T, class... Args>
  int GetMember(ContextTypeRef context, const JniPair<T>& cur,
                const JniPair<Args>&... agrs);

  jobject GetPassThroughObject(const char* field_name,
                               const char* class_signature) {
    jfieldID id = _jni->GetFieldID(_clazz, field_name, class_signature);
    if (id == 0) {
      return nullptr;
    }
    return _jni->GetObjectField(_obj, id);
  }

  template <class T>
  static int GetMember(JNIEnv* jni, jobject java_obj, T& native_obj,
                       ContextTypeRef context);

  std::unique_ptr<JavaToNative> GetObjectMember(const char* name,
                                                const char* type);

 private:
  JNIEnv* _jni = nullptr;
  jobject _obj = nullptr;
  jclass _clazz = nullptr;
};

class NativeToJava final {
 public:
  NativeToJava(JavaVM* jvm, jobject obj);
  ~NativeToJava();
  /**
   * EXAMPLE:
   * java define ==========================================================
   * packge A.B.C
   * public JavaParam {
   *      JavaParam(String name, int[] array) {
   *          mName = name;
   *          mArray = array;
   *      }
   *      public String mName;
   *      int[] mArray;
   * }
   * public JavaTest {
   *      public JavaTest(int a) { ma = a; }
   *      public int testFunc(String param1, JavaParam[][] param2) { }
   *      public void testFunc1(int param) {}
   *      private int ma;
   * };
   *
   * C++ layer
   * ====================================================================
   *
   * struct CppParam {
   *     std::string name;
   *     std::vector<int> array;
   * };
   *
   * CUSTOMER_DEFINE_JNI_SIGANTURE(CppParam, "A/B/C/JavaParam")
   *
   * jobject get_java_param_func(JNIEnv* jni, const CppParam& param,
   *                                  jni_help::ContextTypeRef context) {
   *   jni_help::JavaObjectMaker obj(jni,
   *           jni_help::ClassBaseSignature<CppParam>::signature.c_str(),
   *           context,
   *           CppParam.name,
   *           CppParam.array);
   *   return obj.get_object();
   * }
   *
   * class CppTest {
   * public:
   *   CppTest(int a) {
   *     ::JavaVm* jvm = jni_help::GetJVM();
   *     jni_help::AttachThreadScoped ats(jvm);
   *     JNIEnv* jni = ats.env();
   *     jni_help::JavaObjectMaker obj(jni, "A/B/C/JavaTest", false, a);
   *     _jni_tool.reset(new jni_help::NativeToJava(jvm, obj.get_object()));
   *   }
   *
   *   int TestFunc(std::string param1,
   *                std::vector<std::vector<CppParam>> param2) {
   *      int ret = -1;
   *      if (_jni_tool->CallFunction("testFunc", ret, param1, param2)
   *                    < 0 || ret < 0) {
   *          return -1;
   *      }
   *
   *     <your code .....>
   *   }
   *
   *   void TestFunc1(int param) {
   *     if (_jni_tool->CallVoidFunction("testFunc1", param) < 0) {
   *         return;
   *     }
   *
   *     <your code ....>
   *   }
   * private:
   *   std::unique_ptr<jni_help::NativeToJava> _jni_tool;
   * };
   *
   * */

  template <class... Args>
  int CallVoidFunction(const char* function_name, const Args&... args);

  int CallVoidFunction(const char* function_name);

  template <class ReturnType, class... Args>
  int CallFunction(const char* function_name, ReturnType& return_val,
                   const Args&... args);

  template <class ReturnType>
  int CallFunction(const char* function_name, ReturnType& return_val);

 private:
  JavaVM* _jvm = nullptr;
  jobject _obj = nullptr;
  jclass _clazz = nullptr;
  std::unordered_map<std::string, jmethodID> _method_map;
};

template <class T, class... Args>
int JavaToNative::GetMember(ContextTypeRef context, const char** name,
                            size_t name_size, T& cur, Args&... args) {
  if (name_size != 1 + sizeof...(args) || nullptr == _jni || nullptr == _obj) {
    return -1;
  }
  jfieldID id = _jni->GetFieldID(_clazz, *name,
                                 TraitSignature<T>::signature_trait().c_str());
  if (0 == id || FunctionTraitInternal<T>::get_native_member_func(
                     _jni, _obj, id, cur, context) < 0) {
    return -1;
  }
  if (1 == name_size) {
    return 0;
  }
  return GetMember(context, name + 1, name_size - 1, args...);
}

template <class T, class... Args>
int JavaToNative::GetMember(ContextTypeRef context, const JniPair<T>& cur,
                            const JniPair<Args>&... agrs) {
  if (nullptr == _jni || nullptr == _obj) {
    return -1;
  }
  jfieldID id = _jni->GetFieldID(_clazz, cur.key.c_str(),
                                 TraitSignature<T>::signature_trait().c_str());
  if (0 == id || FunctionTraitInternal<T>::get_native_member_func(
                     _jni, _obj, id, cur.value, context) < 0) {
    return -1;
  }
  return GetMember(context, agrs...);
}
template <class T>
int JavaToNative::GetMember(JNIEnv* jni, jobject java_obj, T& native_obj,
                            ContextTypeRef context) {
  if (nullptr == java_obj) {
    return 0;
  }
  return FunctionTrait<T>::get_native_member_func(jni, java_obj, native_obj,
                                                  context);
}

template <class... Args>
int NativeToJava::CallVoidFunction(const char* function_name,
                                   const Args&... args) {
  if (nullptr == _jvm || nullptr == _clazz || nullptr == _obj) {
    return -1;
  }
  AttachThreadScoped ats(_jvm);
  JNIEnv* jni = ats.env();
  std::string signature = "(" + function_signature_trait(args...) + ")V";
  jmethodID id = jni->GetMethodID(_clazz, function_name, signature.c_str());
  if (0 == id) {
    RTC_DCHECK(false) << "Get method id failed";
    return -1;
  }
  ContextType context;
  jni->CallVoidMethod(
      _obj, id,
      FunctionTraitInternal<Args>::get_java_member_func(jni, args, context)...);
  return 0;
}

template <class ReturnType>
int NativeToJava::CallFunction(const char* function_name,
                               ReturnType& return_val) {
  if (nullptr == _jvm || nullptr == _clazz || nullptr == _obj) {
    return -1;
  }
  AttachThreadScoped ats(_jvm);
  JNIEnv* jni = ats.env();
  std::string signature = "()" + TraitSignature<ReturnType>::signature_trait();
  jmethodID id = jni->GetMethodID(_clazz, function_name, signature.c_str());
  if (0 == id) {
    return -1;
  }
  ContextType context;
  FunctionTrait<ReturnType>::get_native_member_func(
      jni, CallJavaFuntionTrait<ReturnType>::call_java_function(jni, _obj, id),
      return_val, context);
  return 0;
}

template <class ReturnType, class... Args>
int NativeToJava::CallFunction(const char* function_name,
                               ReturnType& return_val, const Args&... args) {
  if (nullptr == _jvm || nullptr == _clazz || nullptr == _obj) {
    return -1;
  }
  AttachThreadScoped ats(_jvm);
  JNIEnv* jni = ats.env();
  std::string signature = "(" + function_signature_trait(args...) + ")" +
                          TraitSignature<ReturnType>::signature_trait();
  jmethodID id = jni->GetMethodID(_clazz, function_name, signature.c_str());
  if (0 == id) {
    return -1;
  }
  ContextType context;

  FunctionTrait<ReturnType>::get_native_member_func(
      jni,
      CallJavaFuntionTrait<ReturnType>::call_java_function(
          jni, _obj, id,
          FunctionTraitInternal<Args>::get_java_member_func(jni, args,
                                                            context)...),
      return_val, context);
  return 0;
}

template <class... Args>
int CallJavaStaticVoidFunction(jclass clazz, const char* function_name,
                               const Args&... args) {
  RTC_DCHECK(clazz != nullptr);
  AttachThreadScoped ats(jni_help::GetJVM());
  JNIEnv* jni = ats.env();
  std::string signature = "(" + function_signature_trait(args...) + ")V";
  jmethodID id = jni->GetMethodID(clazz, function_name, signature.c_str());
  if (0 == id) {
    RTC_DCHECK(false) << "Get method id failed";
    return -1;
  }
  ContextType context;
  jni->CallStaticVoidMethod(
      clazz, id,
      FunctionTraitInternal<Args>::get_java_member_func(jni, args, context)...);
  return 0;
}

int CallJavaStaticVoidFunction(jclass clazz, const char* function_name);

template <class ReturnType, class... Args>
int CallJavaStaticFunction(jclass clazz, const char* function_name,
                           ReturnType& return_val, const Args&... args) {
  RTC_DCHECK(clazz != nullptr);
  AttachThreadScoped ats(GetJVM());
  JNIEnv* jni = ats.env();
  std::string signature = "(" + function_signature_trait(args...) + ")" +
                          TraitSignature<ReturnType>::signature_trait();
  jmethodID id =
      jni->GetStaticMethodID(clazz, function_name, signature.c_str());
  if (0 == id) {
    return -1;
  }
  ContextType context;
  FunctionTrait<ReturnType>::get_native_member_func(
      jni,
      CallJavaFuntionTrait<ReturnType>::call_java_static_function(
          jni, clazz, id,
          FunctionTraitInternal<Args>::get_java_member_func(jni, args,
                                                            context)...),
      return_val, context);

  return 0;
}

template <class ReturnType>
int CallJavaStaticFunction(jclass clazz, const char* function_name,
                           ReturnType& return_val) {
  RTC_DCHECK(clazz != nullptr);
  AttachThreadScoped ats(GetJVM());
  JNIEnv* jni = ats.env();
  std::string signature = "()" + TraitSignature<ReturnType>::signature_trait();
  jmethodID id =
      jni->GetStaticMethodID(clazz, function_name, signature.c_str());
  RTC_DCHECK(id != 0);
  ContextType context;
  FunctionTrait<ReturnType>::get_native_member_func(
      jni,
      CallJavaFuntionTrait<ReturnType>::call_java_static_function(jni, clazz,
                                                                  id),
      return_val, context);

  return 0;
}

template <class... Args>
jobject GetJavaObjectStatic(jclass clazz, const char* func_name,
                            const std::string java_obj_signature,
                            const Args... args) {
  RTC_DCHECK(clazz != nullptr);
  AttachThreadScoped ats(GetJVM());
  JNIEnv* jni = ats.env();
  std::string signature =
      "(" + function_signature_trait(args...) + ")L" + java_obj_signature + ";";
  jmethodID id = jni->GetStaticMethodID(clazz, func_name, signature.c_str());
  if (0 == id) {
    return nullptr;
  }
  ContextType context;
  return CallStaticObjectFunc(
      jni, clazz, id,
      FunctionTraitInternal<Args>::get_java_member_func(jni, args, context)...);

  return nullptr;
}

jobject GetJavaInstance(const std::string& class_signature,
                        const std::string& get_instance_func_name,
                        const std::string& java_obj_signature);

jobject GetJavaInstance(JNIEnv* jni,
    const std::string& class_signature,
    const std::string& get_instance_func_name,
    const std::string& java_obj_signature);

jobject GetJavaObjectStatic(jclass clazz, const std::string& func_name,
                            const std::string& java_obj_signature);

void CallJavaFunctionWithObj(JNIEnv* jni, jobject obj, const char* function_name);

}  // namespace jni_help
#endif // JNI_COMMON_JNI_HELP_TOOL_H_
