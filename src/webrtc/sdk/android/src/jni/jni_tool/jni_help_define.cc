#include "jni_help_define.h"
namespace jni_help {

CUSTOMER_DEFINE_JNI_SIGANTURE(std::string, "java/lang/String")
CUSTOMER_DEFINE_JNI_SIGANTURE(const char*, "java/lang/String")

#define CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(type, str) \
  template <>                                             \
  const std::string jni_help::TypeTraitSignature<type>::signature = str;

CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jint, "I")
CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jboolean, "Z")
CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jbyte, "B")
CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jchar, "C")
CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jshort, "S")
CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jlong, "J")
CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jdouble, "D")
CUSTOMER_DEFINE_BUILT_IN_JNI_SIGANTURE(jfloat, "F")

//============================ function trait ====================================================//
//============================ function trait ====================================================//
jobject CallObjectFunc(JNIEnv* jni, jobject obj, jmethodID id, ...) {
  jni_help::TypeTraitSignature<int>::signature_trait();
  va_list argp;
  va_start(argp, id);
  jobject return_obj = jni->CallObjectMethod(obj, id, argp);
  va_end(argp);
  return return_obj;
}

jobject CallStaticObjectFunc(JNIEnv* jni, jclass clazz, jmethodID id, ...) {
  va_list argp;
  va_start(argp, id);
  jobject return_obj = jni->CallStaticObjectMethod(clazz, id, argp);
  va_end(argp);
  return return_obj;
}

#define ImplementJNIBuiltInFunctionTraitInternal(type_name, member_type)       \
  inline TypeToJniType<member_type>::jni_type                                  \
        Call##type_name##Func(JNIEnv* jni, jobject obj, jmethodID id, ...) {   \
    va_list argp;                                                              \
    va_start(argp, id);                                                        \
    auto return_val = jni->Call##type_name##Method(obj, id, argp);             \
    va_end(argp);                                                              \
    return return_val;                                                         \
  }                                                                            \
  inline TypeToJniType<member_type>::jni_type CallStatic##type_name##Func(     \
      JNIEnv* jni, jclass clazz, jmethodID id, ...) {                          \
    va_list argp;                                                              \
    va_start(argp, id);                                                        \
    auto return_val = jni->CallStatic##type_name##Method(clazz, id, argp);     \
    va_end(argp);                                                              \
    return return_val;                                                         \
  }                                                                            \
                                                                               \
  inline int Get##type_name##Field(JNIEnv* jni, jobject obj, jfieldID id,      \
                                   member_type& member,                        \
                                   ContextTypeRef context) {                   \
    member = jni->Get##type_name##Field(obj, id);                              \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  inline int Get##type_name##ArrayElements(                                    \
      JNIEnv* jni,                                                             \
      typename TypeToJniType<member_type>::jni_array_type& j_array,            \
      member_type*& array_ptr) {                                               \
    array_ptr = jni->Get##type_name##ArrayElements(j_array, 0);                \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  inline void Release##type_name##ArrayElements(                               \
      JNIEnv* jni,                                                             \
      typename TypeToJniType<member_type>::jni_array_type& j_array,            \
      member_type*& array_ptr) {                                               \
    jni->Release##type_name##ArrayElements(j_array, array_ptr, 0);             \
    array_ptr = nullptr;                                                       \
  }                                                                            \
                                                                               \
  inline void Set##type_name##ArrayRegion(                                     \
      JNIEnv* jni,                                                             \
      typename TypeToJniType<member_type>::jni_array_type& j_array,            \
      jsize start_pos, jsize size, const member_type* array_ptr) {             \
    jni->Set##type_name##ArrayRegion(j_array, start_pos, size, array_ptr);     \
  }                                                                            \
                                                                               \
 std::function<int(JNIEnv*,                                                    \
     typename TypeToJniType<member_type>::jni_type,                            \
                          member_type&, ContextTypeRef)>                       \
  FunctionTrait<member_type>::get_native_member_func =                         \
      [] (JNIEnv*,                                                             \
          typename TypeToJniType<member_type>::jni_type jni_val,               \
          member_type& c_val, ContextTypeRef) -> int {                         \
        c_val = jni_val;                                                       \
        return 0;                                                              \
      };                                                                       \
                                                                               \
                                                                               \
  Call##type_name##FuncType                                                    \
      CallJavaFuntionTrait<member_type>::call_java_function =                  \
          Call##type_name##Func;                                               \
  CallStatic##type_name##FuncType                                              \
      CallJavaFuntionTrait<member_type>::call_java_static_function =           \
          CallStatic##type_name##Func;                                         \
                                                                               \
  std::function<int(                                                           \
      JNIEnv * jni,                                                            \
      typename TypeToJniType<member_type>::jni_array_type & j_array,           \
      member_type * &array_ptr)>                                               \
      JniArrayFunctionTrait<member_type>::get_array_func =                     \
          Get##type_name##ArrayElements;                                       \
                                                                               \
  std::function<void(                                                          \
      JNIEnv * jni,                                                            \
      typename TypeToJniType<member_type>::jni_array_type & j_array,           \
      member_type * &array_ptr)>                                               \
      JniArrayFunctionTrait<member_type>::release_array_func =                 \
          Release##type_name##ArrayElements;                                   \
                                                                               \
  std::function<void(                                                          \
      JNIEnv * jni,                                                            \
      typename TypeToJniType<member_type>::jni_array_type & j_array, jsize,    \
      jsize, const member_type* array_ptr)>                                    \
      JniArrayFunctionTrait<member_type>::set_array_func =                     \
          Set##type_name##ArrayRegion;                                         \
                                                                               \
  std::function<int(JNIEnv*, jobject, jfieldID, member_type&, ContextTypeRef)> \
      FunctionTraitInternal<member_type, void>::get_native_member_func =       \
          Get##type_name##Field;                                               \
                                                                               \
  std::function<TypeToJniType<member_type>::jni_type(JNIEnv*, member_type,     \
                                                     ContextTypeRef)>          \
      FunctionTraitInternal<member_type, void>::get_java_member_func =         \
          [](JNIEnv*, member_type member, ContextTypeRef) { return member; };  \
                                                                               \
  typename TypeToJniType<CStyleArray<member_type>>::jni_type set_array_func(   \
      JNIEnv* jni, const member_type* ptr, size_t size) {                      \
    typename TypeToJniType<CStyleArray<member_type>>::jni_type j_array =       \
        nullptr;                                                               \
    if (nullptr != ptr && size > 0) {                                          \
      j_array = jni->New##type_name##Array(size);                              \
      JniArrayFunctionTrait<member_type>::set_array_func(jni, j_array, 0,      \
                                                         size, ptr);           \
    }                                                                          \
    return j_array;                                                            \
  }                                                                            \
                                                                               \
  std::function<int(JNIEnv*, jobject, CStyleArrayRef<member_type>&,            \
                    ContextTypeRef)>                                           \
      FunctionTrait<CStyleArrayRef<member_type>>::get_native_member_func =     \
          [](JNIEnv* jni, jobject obj, CStyleArrayRef<member_type>& out,       \
             ContextTypeRef context) {                                         \
            TypeToJniType<member_type>::jni_array_type j_array =               \
                reinterpret_cast<TypeToJniType<member_type>::jni_array_type>(  \
                    obj);                                                      \
            auto native_array =                                                \
                context.make_ptr<JavaArrayToNative<member_type>>();            \
            if (nullptr != j_array) {                                          \
              native_array->reset(jni, j_array);                               \
            }                                                                  \
            out.ptr = native_array->get_data_ptr();                            \
            out.size = native_array->get_data_size();                          \
            return 0;                                                          \
          };                                                                   \
                                                                               \
  std::function<int(JNIEnv*, jobject, CStyleArray<member_type>&,               \
                    ContextTypeRef)>                                           \
      FunctionTrait<CStyleArray<member_type>>::get_native_member_func =        \
          [](JNIEnv* jni, jobject obj, CStyleArray<member_type>& out,          \
             ContextTypeRef context) {                                         \
            TypeToJniType<member_type>::jni_array_type j_array =               \
                reinterpret_cast<TypeToJniType<member_type>::jni_array_type>(  \
                    obj);                                                      \
            if (nullptr != j_array) {                                          \
              JavaArrayToNative<member_type> array_tmp(jni, j_array);          \
              return array_tmp.fill_array(out.ptr, out.size);                  \
            }                                                                  \
            return 0;                                                          \
          };                                                                   \
                                                                               \
  std::function<typename TypeToJniType<CStyleArray<member_type>>::jni_type(    \
      JNIEnv*, const CStyleArray<member_type>&, ContextTypeRef)>               \
      FunctionTrait<CStyleArray<member_type>>::get_java_member_func =          \
          [](JNIEnv* jni, const CStyleArray<member_type>& cs_array,            \
             ContextTypeRef context) {                                         \
            auto j_array =                                                     \
                set_array_func(jni, cs_array.const_ptr, cs_array.size);        \
            return j_array;                                                    \
          };                                                                   \
                                                                               \
  std::function<int(JNIEnv*, jobject, std::vector<member_type>&,               \
                    ContextTypeRef)>                                           \
      FunctionTrait<std::vector<member_type>>::get_native_member_func =        \
          [](JNIEnv* jni, jobject obj, std::vector<member_type>& out,          \
             ContextTypeRef context) {                                         \
            TypeToJniType<member_type>::jni_array_type j_array =               \
                reinterpret_cast<TypeToJniType<member_type>::jni_array_type>(  \
                    obj);                                                      \
            if (nullptr != j_array) {                                          \
              JavaArrayToNative<member_type> array_tmp(jni, j_array);          \
              return array_tmp.fill_array(out);                                \
            }                                                                  \
            return 0;                                                          \
          };                                                                   \
                                                                               \
  std::function<typename TypeToJniType<std::vector<member_type>>::jni_type(    \
      JNIEnv*, const std::vector<member_type>&, ContextTypeRef)>               \
      FunctionTrait<std::vector<member_type>>::get_java_member_func =          \
          [](JNIEnv* jni, const std::vector<member_type>& v,                   \
             ContextTypeRef context) ->                                        \
      typename TypeToJniType<std::vector<member_type>>::jni_type {             \
        if (v.size() == 0) {                                                   \
          return nullptr;                                                      \
        }                                                                      \
        auto j_array = set_array_func(jni, &v[0], v.size());                   \
        return j_array;                                                        \
      };


ImplementJNIBuiltInFunctionTraitInternal(Int, int)
ImplementJNIBuiltInFunctionTraitInternal(Byte, int8_t)
ImplementJNIBuiltInFunctionTraitInternal(Char, uint16_t)
ImplementJNIBuiltInFunctionTraitInternal(Short, int16_t)
ImplementJNIBuiltInFunctionTraitInternal(Float, float)
ImplementJNIBuiltInFunctionTraitInternal(Long, int64_t)
ImplementJNIBuiltInFunctionTraitInternal(Double, double)
ImplementJNIBuiltInFunctionTraitInternal(Boolean, uint8_t)

const std::function<int(JNIEnv*, jobject, jobject&,
        ContextTypeRef)> FunctionTrait<jobject>::get_native_member_func =
            [](JNIEnv* jni, jobject obj, jobject& out,
                                       ContextTypeRef context) -> int {
  out = obj;
  return 0;
};

const std::function<jobject(JNIEnv*, const jobject&, ContextTypeRef)>
    FunctionTrait<jobject>::get_java_member_func =
        [](JNIEnv* jni, const jobject& obj,
           ContextTypeRef context) -> jobject {
  return obj;
};

const std::function<int(JNIEnv*, jobject, std::string&,
        ContextTypeRef)> FunctionTrait<std::string>::get_native_member_func =
            [](JNIEnv* jni, jobject obj, std::string& out,
                                       ContextTypeRef context) -> int {
  JavaArrayToNative<char> array_str(jni, reinterpret_cast<jstring>(obj));
  out = std::string(array_str.get_data_ptr());
  return 0;
};

const std::function<jstring(JNIEnv*, const std::string&, ContextTypeRef)>
    FunctionTrait<std::string>::get_java_member_func =
        [](JNIEnv* jni, const std::string& str,
           ContextTypeRef context) -> jstring {
  jstring j_string = jni->NewStringUTF(str.c_str());
  return j_string;
};

template <>
const std::function<int(JNIEnv*, jobject, const char*&, ContextTypeRef)>
    FunctionTrait<const char*>::get_native_member_func =
        [](JNIEnv* jni, jobject obj, const char*& out,
           ContextTypeRef context) -> int {
  auto char_array = context.make_ptr<JavaArrayToNative<char>>();
  char_array->reset(jni, reinterpret_cast<jstring>(obj));
  out = char_array->get_data_ptr();
  return 0;
};

std::function<int(JNIEnv*, jobject, CStyleArray<char>&, ContextTypeRef)>
    FunctionTrait<CStyleArray<char>>::get_native_member_func =
        [](JNIEnv* jni, jobject obj, CStyleArray<char>& out,
           ContextTypeRef context) {
          jstring j_array = reinterpret_cast<jstring>(obj);
          if (nullptr != j_array) {
            JavaArrayToNative<char> array_tmp(jni, j_array);
            return array_tmp.fill_array(out.ptr, out.size);
          }
          return 0;
        };

template <>
const std::function<jstring(JNIEnv*, const char* const&, ContextTypeRef)>
    FunctionTrait<const char*>::get_java_member_func =
        [](JNIEnv* jni, const char* const& str,
           ContextTypeRef context) -> jstring {
  if (nullptr == str) {
    return nullptr;
  }
  jstring j_string = jni->NewStringUTF(str);
  return j_string;
};

std::function<int(JNIEnv*, jobject, jfieldID, bool&, ContextTypeRef)>
    FunctionTraitInternal<bool, void>::get_native_member_func =
        [](JNIEnv* jni, jobject obj, jfieldID id, bool& out,
           ContextTypeRef context) -> int {
  uint8_t tmp = 0;
  if (FunctionTraitInternal<uint8_t>::get_native_member_func(jni, obj, id, tmp,
                                                             context) < 0) {
    return -1;
  }
  out = tmp > 0;
  return 0;
};

std::function<int(JNIEnv*, jobject, jfieldID, uint32_t&, ContextTypeRef)>
    FunctionTraitInternal<uint32_t, void>::get_native_member_func =
        [](JNIEnv* jni, jobject obj, jfieldID id, uint32_t& out,
           ContextTypeRef context) {
          int32_t tmp = 0;
          if (FunctionTraitInternal<int32_t, void>::get_native_member_func(
                  jni, obj, id, tmp, context) < 0) {
            return -1;
          }
          out = static_cast<uint32_t>(tmp);
          return 0;
        };

std::function<jint(JNIEnv*, uint32_t, ContextTypeRef)>
    FunctionTraitInternal<uint32_t, void>::get_java_member_func =
        [](JNIEnv*, uint32_t in, ContextTypeRef) {
          return static_cast<jint>(in);
        };

std::function<int(JNIEnv*, jobject, jfieldID, uint64_t&, ContextTypeRef)>
    FunctionTraitInternal<uint64_t, void>::get_native_member_func =
        [](JNIEnv* jni, jobject obj, jfieldID id, uint64_t& out,
           ContextTypeRef context) {
          int64_t tmp = 0;
          if (FunctionTraitInternal<int64_t, void>::get_native_member_func(
                  jni, obj, id, tmp, context) < 0) {
            return -1;
          }
          out = static_cast<uint64_t>(tmp);
          return 0;
        };

std::function<jlong(JNIEnv*, uint64_t, ContextTypeRef)>
    FunctionTraitInternal<uint64_t, void>::get_java_member_func =
        [](JNIEnv*, uint64_t in, ContextTypeRef) {
          return static_cast<int64_t>(in);
        };


//============================ function trait ====================================================//
//============================ function trait ====================================================//
}  // namespace jni_help
