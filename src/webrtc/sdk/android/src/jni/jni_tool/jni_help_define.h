#ifndef JNI_COMMON_JNI_HELP_DEFINE_H_
#define JNI_COMMON_JNI_HELP_DEFINE_H_

#include <jni.h>

#include <functional>
#include <string>
#include <type_traits>
#include <vector>
#include "rtc_base/checks.h"

namespace jni_help {

jclass FindClass(JNIEnv* jni, const char* name);
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define ARRAY_SIZE(array) (sizeof(jni_help::ArraySizeHelper(array)))

template <class T>
struct CStyleArrayRef {
  CStyleArrayRef(const T*& ptr_ptr, int& ptr_size)
      : ptr(ptr_ptr), size(ptr_size) {}
  const T*& ptr;
  int& size;
};

template <class T>
struct CStyleArray {
  CStyleArray(const T* ptr_ptr, size_t ptr_size)
      : const_ptr(ptr_ptr), size(ptr_size) {}
  CStyleArray(T* ptr_ptr, size_t ptr_size) : ptr(ptr_ptr), size(ptr_size) {}
  union {
    const T* const_ptr;
    T* ptr;
  };
  size_t size;
};

//============================ ResouceManager ====================================================//
//============================ ResouceManager ====================================================//
constexpr size_t kMaxJavaRefSizeInVector = 20;
class ResouceManager final {
 public:
  ResouceManager() = default;
  ~ResouceManager() {
    ClearJavaRef();
    ClearCppMem();
  }
  template <class T, class... Args>
  T* make_ptr(Args... args);
  template <class T>
  T* make_ptr();
  template <class T>
  T* make_arr_ptr(size_t size);

  template <class T>
  void delete_local_ref(JNIEnv* jni, T obj);

 private:
  void ClearJavaRef() {
    for (auto& ee : func_v_java_ref_) {
      ee();
    }
    func_v_java_ref_.clear();
  }
  void ClearCppMem() {
    for (auto& ee : func_v_cpp_mem_) {
      ee();
    }
    func_v_cpp_mem_.clear();
  }
  std::vector<std::function<void()>> func_v_cpp_mem_;
  std::vector<std::function<void()>> func_v_java_ref_;
};

template <class T, class... Args>
T* ResouceManager::make_ptr(Args... args) {
  T* ptr = new T(std::forward<Args>(args)...);
  func_v_cpp_mem_.push_back([ptr]() { delete ptr; });
  return ptr;
}

template <class T>
T* ResouceManager::make_ptr() {
  T* ptr = new T();
  func_v_cpp_mem_.push_back([ptr]() { delete ptr; });
  return ptr;
}

template <class T>
T* ResouceManager::make_arr_ptr(size_t size) {
  T* ptr = new T[size];
  func_v_cpp_mem_.push_back([ptr]() { delete[] ptr; });
  return ptr;
}

template <class T>
void ResouceManager::delete_local_ref(JNIEnv* jni, T obj) {
  if (func_v_java_ref_.size() > kMaxJavaRefSizeInVector) {
    ClearJavaRef();
  }
  if (nullptr != obj) {
    func_v_java_ref_.push_back([jni, obj] { jni->DeleteLocalRef(obj); });
  }
}

using ContextType = ResouceManager;
using ContextTypeRef = ContextType&;
//============================ ResouceManager ====================================================//
//============================ ResouceManager ====================================================//

template <class T, class V = void>
class JavaArrayToNative {
  static_assert(sizeof(T) < 0, "JavaArrayToNative unsupport type");
};

//============================ type trait ========================================================//
//============================ type trait ========================================================//
template <class T>
struct TypeToJniType {
  typedef jobject jni_type;
  typedef jobjectArray jni_array_type;
};

template <>
struct TypeToJniType<std::string> {
  typedef jstring jni_type;
};

template <>
struct TypeToJniType<const char*> {
  typedef jstring jni_type;
};

template <class T>
struct IsJniBuiltInType : public std::false_type {};

#define DefineTypeToJniType(type, jtype)                    \
  template <>                                               \
  struct IsJniBuiltInType<type> : public std::true_type {}; \
                                                            \
  template <>                                               \
  struct TypeToJniType<type> {                              \
    typedef jtype jni_type;                                 \
    typedef jtype##Array jni_array_type;                    \
  };                                                        \
  template <>                                               \
  struct TypeToJniType<std::vector<type>> {                 \
    typedef TypeToJniType<type>::jni_array_type jni_type;   \
  };                                                        \
  template <>                                               \
  struct TypeToJniType<CStyleArrayRef<type>>                \
      : public TypeToJniType<std::vector<type>> {};         \
  template <>                                               \
  struct TypeToJniType<CStyleArray<type>>                   \
      : public TypeToJniType<std::vector<type>> {};

DefineTypeToJniType(int8_t, jbyte)
DefineTypeToJniType(uint8_t, jboolean)
DefineTypeToJniType(uint16_t, jchar)
DefineTypeToJniType(int16_t, jshort)
DefineTypeToJniType(int, jint)
DefineTypeToJniType(uint32_t, jint)
DefineTypeToJniType(int64_t, jlong)
DefineTypeToJniType(uint64_t, jlong)
DefineTypeToJniType(float, jfloat)
DefineTypeToJniType(double, jdouble)

template <class T>
struct ClassBaseSignature {
  static std::string signature_trait() { return signature; }
 private:
  static const std::string signature;
};

template <class T>
struct TypeTraitSignature {
  static const std::string& signature_trait() {
    RTC_DCHECK(signature != "");
    return signature;
  }

 private:
  static const std::string signature;
};

// template<class T>
// const std::string TypeTraitSignature<T>::signature = "";

template <class T>
struct TypeTraitSignature<std::vector<T>> {
  static std::string signature_trait() {
    return "[" + TypeTraitSignature<T>::signature_trait();
  }
};

#define CUSTOMER_DEFINE_JNI_SIGANTURE(type, str)                         \
  template <>                                                            \
  const std::string jni_help::ClassBaseSignature<type>::signature = str; \
  template <>                                                            \
  const std::string jni_help::TypeTraitSignature<type>::signature =      \
      std::string("L") + str + ";";

template <class T>
struct ClassBaseSignature<std::vector<T>>
    : public TypeTraitSignature<std::vector<T>> {};

template <class T>
struct ClassBaseSignature<CStyleArrayRef<T>>
    : public ClassBaseSignature<std::vector<T>> {};

template <class T>
struct ClassBaseSignature<CStyleArray<T>>
    : public ClassBaseSignature<std::vector<T>> {};

template <class T>
struct TypeTraitSignature<CStyleArrayRef<T>>
    : public TypeTraitSignature<std::vector<T>> {};

template <class T>
struct TypeTraitSignature<CStyleArray<T>>
    : public TypeTraitSignature<std::vector<T>> {};

template <class T, class V = void>
struct TraitSignature : public TypeTraitSignature<T> {};

template <class T>
struct TraitSignature<T,
                      typename std::enable_if<IsJniBuiltInType<T>::value>::type>
    : public TypeTraitSignature<typename TypeToJniType<T>::jni_type> {};

template <class T>
struct TraitSignature<T, typename std::enable_if<std::is_enum<T>::value>::type>
    : public TypeTraitSignature<jint> {};

template <>
struct TraitSignature<bool, void> : public TypeTraitSignature<jboolean> {};

template <>
struct TraitSignature<CStyleArray<char>, void>
    : public TypeTraitSignature<std::string> {};

inline std::string function_signature_trait() { return ""; }

template <class Cur, class... Args>
std::string function_signature_trait(const Cur&, const Args&... args) {
  return TraitSignature<Cur>::signature_trait() +
         function_signature_trait(args...);
}

//============================ type trait ========================================================//
//============================ type trait ========================================================//

//============================ function trait ====================================================//
//============================ function trait ====================================================//

template <class T>
struct FunctionTrait {
  static const std::function<int(JNIEnv*, jobject, T&, ContextTypeRef)>
      get_native_member_func;
  static const std::function<typename TypeToJniType<T>::jni_type(JNIEnv*, const T&,
                                                           ContextTypeRef)>
      get_java_member_func;
};

template<>
struct FunctionTrait<jobject> {
  static const std::function<int(JNIEnv*, jobject, jobject&, ContextTypeRef)>
      get_native_member_func;
  static const std::function<jobject(JNIEnv*, const jobject&, ContextTypeRef)>
      get_java_member_func;
};

template <>
struct FunctionTrait<std::string> {
  static const std::function<int(JNIEnv*, jobject, std::string&, ContextTypeRef)>
      get_native_member_func;
  static const std::function<jstring(JNIEnv*, const std::string&, ContextTypeRef)>
      get_java_member_func;
};

#define CUSTOMER_DEFINE_JNI_NATIVE_FUNCTION_TRAIT(type, func)           \
  template <>                                                           \
  const std::function<int(JNIEnv*, jobject, type&, jni_help::ContextTypeRef)> \
      jni_help::FunctionTrait<type>::get_native_member_func = func;

#define CUSTOMER_DEFINE_JNI_JAVA_FUNCTION_TRAIT(type, func)              \
  template <>                                                            \
  const std::function<jobject(JNIEnv*, const type&, jni_help::ContextTypeRef)> \
      jni_help::FunctionTrait<type>::get_java_member_func = func;

typedef jobject (*CallObjectFuncType)(JNIEnv*, jobject, jmethodID, ...);
typedef jobject (*CallStaticObjectFuncType)(JNIEnv*, jclass, jmethodID,
                                         ...);

jobject CallObjectFunc(JNIEnv*, jobject, jmethodID, ...);
jobject CallStaticObjectFunc(JNIEnv*, jclass, jmethodID, ...);
template <class T>
struct CallJavaFuntionTrait {
  static CallObjectFuncType call_java_function;
  static CallStaticObjectFuncType call_java_static_function;
};

template <class T>
CallObjectFuncType CallJavaFuntionTrait<T>::call_java_function = CallObjectFunc;
template <class T>
CallStaticObjectFuncType CallJavaFuntionTrait<T>::call_java_static_function =
    CallStaticObjectFunc;

template <class T, class V = void>
struct FunctionTraitInternal {
  static std::function<int(JNIEnv*, jobject, jfieldID, T&, ContextTypeRef)>
      get_native_member_func;
  static std::function<typename TypeToJniType<T>::jni_type(JNIEnv*, const T&,
                                                           ContextTypeRef)>
      get_java_member_func;
};
template <class T, class V>
std::function<int(JNIEnv*, jobject, jfieldID, T&, ContextTypeRef)>
    FunctionTraitInternal<T, V>::get_native_member_func =
        [](JNIEnv* jni, jobject j_obj, jfieldID j_id, T& out,
           ContextTypeRef context) -> int {
  jobject obj = jni->GetObjectField(j_obj, j_id);
  if (nullptr == obj) {
    return 0;
  }
  context.delete_local_ref(jni, obj);
  return FunctionTrait<T>::get_native_member_func(jni, obj, out, context);
};

template <class T, class V>
std::function<typename TypeToJniType<T>::jni_type(JNIEnv*, const T&,
                                                  ContextTypeRef)>
    FunctionTraitInternal<T, V>::get_java_member_func =
        [](JNIEnv* jni, const T& obj, ContextTypeRef context) {
          return FunctionTrait<T>::get_java_member_func(jni, obj, context);
        };

template <class T>
struct JniArrayFunctionTrait {
  static_assert(sizeof(T) < 0, "JniArrayFunctionTrait unsupport type");
};

#define DefineJNIBuiltInFunctionTraitInternal(type_name, member_type)          \
  typedef TypeToJniType<member_type>::jni_type                                 \
          (*Call##type_name##FuncType)(JNIEnv*, jobject, jmethodID, ...);      \
  typedef TypeToJniType<member_type>::jni_type                                 \
          (*CallStatic##type_name##FuncType)(JNIEnv*, jclass, jmethodID, ...); \
                                                                               \
  template <>                                                                  \
  struct CallJavaFuntionTrait<member_type> {                                   \
    static Call##type_name##FuncType call_java_function;                       \
    static CallStatic##type_name##FuncType call_java_static_function;          \
  };                                                                           \
                                                                               \
  template <>                                                                  \
  struct JniArrayFunctionTrait<member_type> {                                  \
    static std::function<int(                                                  \
        JNIEnv * jni,                                                          \
        typename TypeToJniType<member_type>::jni_array_type& j_array,          \
        member_type*& array_ptr)>                                              \
        get_array_func;                                                        \
    static std::function<void(                                                 \
        JNIEnv * jni,                                                          \
        typename TypeToJniType<member_type>::jni_array_type& j_array,          \
        member_type*& array_ptr)>                                              \
        release_array_func;                                                    \
    static std::function<void(                                                 \
        JNIEnv * jni,                                                          \
        typename TypeToJniType<member_type>::jni_array_type& j_array, jsize,   \
        jsize, const member_type* array_ptr)>                                  \
        set_array_func;                                                        \
  };                                                                           \
                                                                               \
  template <>                                                                  \
  struct FunctionTraitInternal<member_type, void> {                            \
    static std::function<int(JNIEnv*, jobject, jfieldID, member_type&,         \
                             ContextTypeRef)>                                  \
        get_native_member_func;                                                \
    static std::function<TypeToJniType<member_type>::jni_type(                 \
        JNIEnv*, member_type, ContextTypeRef)>                                 \
        get_java_member_func;                                                  \
  };                                                                           \
                                                                               \
  template <>                                                                  \
  struct FunctionTrait<member_type> {                                          \
    static std::function<int(JNIEnv*,                                          \
        typename TypeToJniType<member_type>::jni_type,                         \
                             member_type&, ContextTypeRef)>                    \
        get_native_member_func;                                                \
    static std::function<TypeToJniType<member_type>::jni_type(                 \
        JNIEnv*, const member_type&, ContextTypeRef)>                          \
        get_java_member_func;                                                  \
  };                                                                           \
                                                                               \
  template <>                                                                  \
  struct FunctionTrait<CStyleArray<member_type>> {                             \
    static std::function<int(JNIEnv*, jobject, CStyleArray<member_type>&,      \
                             ContextTypeRef)>                                  \
        get_native_member_func;                                                \
    static std::function<TypeToJniType<CStyleArray<member_type>>::jni_type(    \
        JNIEnv*, const CStyleArray<member_type>&, ContextTypeRef)>             \
        get_java_member_func;                                                  \
  };                                                                           \
  template <>                                                                  \
  struct FunctionTrait<CStyleArrayRef<member_type>> {                          \
    static std::function<int(JNIEnv*, jobject, CStyleArrayRef<member_type>&,   \
                             ContextTypeRef)>                                  \
        get_native_member_func;                                                \
    static std::function<TypeToJniType<CStyleArrayRef<member_type>>::jni_type( \
        JNIEnv*, const CStyleArrayRef<member_type>&, ContextTypeRef)>          \
        get_java_member_func;                                                  \
  };                                                                           \
                                                                               \
  template <>                                                                  \
  struct FunctionTrait<std::vector<member_type>> {                             \
    static std::function<int(JNIEnv*, jobject, std::vector<member_type>&,      \
                             ContextTypeRef)>                                  \
        get_native_member_func;                                                \
    static std::function<TypeToJniType<std::vector<member_type>>::jni_type(    \
        JNIEnv*, const std::vector<member_type>&, ContextTypeRef)>             \
        get_java_member_func;                                                  \
  };

template <>
struct FunctionTrait<CStyleArray<char>> {
  static std::function<int(JNIEnv*, jobject, CStyleArray<char>&,
                           ContextTypeRef)>
      get_native_member_func;
};

template <>
struct FunctionTraitInternal<uint32_t, void> {
  static std::function<int(JNIEnv*, jobject, jfieldID, uint32_t&,
                           ContextTypeRef)>
      get_native_member_func;
  static std::function<jint(JNIEnv*, uint32_t, ContextTypeRef)>
      get_java_member_func;
};

template <>
struct FunctionTraitInternal<uint64_t, void> {
  static std::function<int(JNIEnv*, jobject, jfieldID, uint64_t&,
                           ContextTypeRef)>
      get_native_member_func;
  static std::function<jlong(JNIEnv*, uint64_t, ContextTypeRef)>
      get_java_member_func;
};

template <>
struct FunctionTraitInternal<bool, void> {
  static std::function<int(JNIEnv*, jobject, jfieldID, bool&, ContextTypeRef)>
      get_native_member_func;
  static std::function<jlong(JNIEnv*, bool, ContextTypeRef)>
      get_java_member_func;
};

DefineJNIBuiltInFunctionTraitInternal(Int, int)
DefineJNIBuiltInFunctionTraitInternal(Byte, int8_t)
DefineJNIBuiltInFunctionTraitInternal(Char, uint16_t)
DefineJNIBuiltInFunctionTraitInternal(Short, int16_t)
DefineJNIBuiltInFunctionTraitInternal(Float, float)
DefineJNIBuiltInFunctionTraitInternal(Long, int64_t)
DefineJNIBuiltInFunctionTraitInternal(Double, double)
DefineJNIBuiltInFunctionTraitInternal(Boolean,uint8_t)

template <class T>
struct FunctionTrait<CStyleArray<T>> {
  static std::function<int(JNIEnv*, jobject, CStyleArray<T>&, ContextTypeRef)>
      get_native_member_func;
  static std::function<jobjectArray(JNIEnv*, const CStyleArray<T>&,
                                    ContextTypeRef)>
      get_java_member_func;
};

template <class T>
struct FunctionTrait<CStyleArrayRef<T>> {
  static std::function<int(JNIEnv*, jobject, CStyleArrayRef<T>&,
                           ContextTypeRef)>
      get_native_member_func;
  static std::function<jobjectArray(JNIEnv*, const CStyleArrayRef<T>&,
                                    ContextTypeRef)>
      get_java_member_func;
};

template <class T>
struct FunctionTrait<std::vector<T>> {
  static std::function<int(JNIEnv*, jobject, std::vector<T>&, ContextTypeRef)>
      get_native_member_func;
  static std::function<jobjectArray(JNIEnv*, const std::vector<T>&,
                                    ContextTypeRef)>
      get_java_member_func;
};

template <class T>
int get_native_array_member_func(JNIEnv* jni, jobjectArray j_array, T* out,
                                 size_t size, ContextTypeRef context) {
  RTC_DCHECK((int)size == jni->GetArrayLength(j_array));
  for (size_t i = 0; i < size; ++i) {
    jobject ele = jni->GetObjectArrayElement(j_array, i);
    if (nullptr == ele) {
      continue;
    }
    if (FunctionTrait<T>::get_native_member_func(jni, ele, out[i], context) <
        0) {
      return -1;
    }
    jni->DeleteLocalRef(ele);
  }
  return 0;
}

template <class T>
jobjectArray get_java_array_member_func_(JNIEnv* jni, const T* ptr, size_t size,
                                         ContextTypeRef context) {
  if (nullptr == ptr || 0 == size) {
    return nullptr;
  }
  jclass clazz =
      FindClass(jni, ClassBaseSignature<T>::signature_trait().c_str());
  jobjectArray obj_array = jni->NewObjectArray(size, clazz, nullptr);
  for (size_t i = 0; i < size; ++i) {
    jobject tmp_obj =
        FunctionTrait<T>::get_java_member_func(jni, ptr[i], context);
    jni->SetObjectArrayElement(obj_array, i, tmp_obj);
    jni->DeleteLocalRef(tmp_obj);
  }
  jni->DeleteLocalRef(clazz);
  return obj_array;
}

template <class T>
std::function<int(JNIEnv*, jobject, CStyleArrayRef<T>&, ContextTypeRef)>
    FunctionTrait<CStyleArrayRef<T>>::get_native_member_func =
        [](JNIEnv* jni, jobject j_obj, CStyleArrayRef<T>& out,
           ContextTypeRef context) -> int {
  jobjectArray j_array = reinterpret_cast<jobjectArray>(j_obj);
  if (nullptr == j_array) {
    return 0;
  }
  size_t size = jni->GetArrayLength(j_array);
  T* tmp = context.make_arr_ptr<T>(size);
  if (get_native_array_member_func(jni, j_array, tmp, size, context) < 0) {
    return -1;
  }
  out.ptr = tmp;
  out.size = size;
  return 0;
};

template <class T>
std::function<int(JNIEnv*, jobject, CStyleArray<T>&, ContextTypeRef)>
    FunctionTrait<CStyleArray<T>>::get_native_member_func =
        [](JNIEnv* jni, jobject j_obj, CStyleArray<T>& out,
           ContextTypeRef context) -> int {
  jobjectArray j_array = reinterpret_cast<jobjectArray>(j_obj);
  if (nullptr == j_array) {
    return 0;
  }
  size_t size = jni->GetArrayLength(j_array);
  if (out.size < size) {
    return -1;
  }
  return get_native_array_member_func(jni, j_array, out.ptr, size, context);
};

template <class T>
std::function<jobjectArray(JNIEnv*, const CStyleArray<T>&, ContextTypeRef)>
    FunctionTrait<CStyleArray<T>>::get_java_member_func =
        [](JNIEnv* jni, const CStyleArray<T>& v,
           ContextTypeRef context) -> jobjectArray {
  if (nullptr == v.const_ptr || 0 == v.size) {
    return nullptr;
  }
  return get_java_array_member_func_(jni, v.const_ptr, v.size, context);
};

template <class T>
std::function<int(JNIEnv*, jobject, std::vector<T>&, ContextTypeRef)>
    FunctionTrait<std::vector<T>>::get_native_member_func =
        [](JNIEnv* jni, jobject j_obj, std::vector<T>& out,
           ContextTypeRef context) -> int {
  jobjectArray j_array = reinterpret_cast<jobjectArray>(j_obj);
  if (nullptr == j_array) {
    return 0;
  }
  size_t size = jni->GetArrayLength(j_array);
  std::vector<T> tmp(size);
  if (get_native_array_member_func(jni, j_array, &tmp[0], size, context) < 0) {
    return -1;
  }
  std::swap(tmp, out);
  return 0;
};

template <class T>
std::function<jobjectArray(JNIEnv*, const std::vector<T>&, ContextTypeRef)>
    FunctionTrait<std::vector<T>>::get_java_member_func =
        [](JNIEnv* jni, const std::vector<T>& v,
           ContextTypeRef context) -> jobjectArray {
  if (v.size() == 0) {
    return nullptr;
  }
  return get_java_array_member_func_(jni, &v[0], v.size(), context);
};

template <class T>
struct FunctionTraitInternal<
    T, typename std::enable_if<std::is_enum<T>::value>::type> {
  static std::function<int(JNIEnv*, jobject, jfieldID, T&, ContextTypeRef)>
      get_native_member_func;
};

template <class T>
std::function<int(JNIEnv*, jobject, jfieldID, T&, ContextTypeRef)>
    FunctionTraitInternal<T, typename std::enable_if<std::is_enum<T>::value>::
                                 type>::get_native_member_func =
        [](JNIEnv* jni, jobject obj, jfieldID id, T& out,
           ContextTypeRef context) -> int {
  int tmp = 0;
  if (FunctionTraitInternal<int>::get_native_member_func(jni, obj, id, tmp,
                                                         context) < 0) {
    return -1;
  }
  out = static_cast<T>(tmp);
  return 0;
};
//============================ function trait ====================================================//
//============================ function trait ====================================================//

//============================ JavaArrayToNative =================================================//
//============================ JavaArrayToNative =================================================//
template <class T>
class JavaArrayToNative<
    T, typename std::enable_if<IsJniBuiltInType<T>::value>::type>
    final {
 public:
  JavaArrayToNative() = default;
  JavaArrayToNative(JNIEnv* jni,
                    typename TypeToJniType<T>::jni_array_type j_array)
      : _jni(jni) {
    reset(jni, j_array);
  }

  ~JavaArrayToNative() { clear(); }

  void reset(JNIEnv* jni, typename TypeToJniType<T>::jni_array_type j_array) {
    if (nullptr == jni || nullptr == j_array) {
      return;
    }
    clear();
    _jni = jni;
    _j_array = static_cast<typename TypeToJniType<T>::jni_array_type>(
        jni->NewGlobalRef(j_array));
    if (JniArrayFunctionTrait<T>::get_array_func(_jni, _j_array, _data) == 0) {
      _size = _jni->GetArrayLength(_j_array);
    }
  }

  int fill_array(std::vector<T>& vec) const {
    if (vec.size() < _size) {
      vec = std::vector<T>(_size);
    }
    return fill_array(&vec[0], _size);
  }

  int fill_array(T* ptr, size_t size) const {
    if (size < _size) {
      return -1;
    }
    memcpy(ptr, _data, sizeof(T) * _size);
    return 0;
  }

  inline const T* get_data_ptr() const { return _data; }

  inline size_t get_data_size() const { return _size; }

 private:
  void clear() {
    if (nullptr != _j_array) {
      JniArrayFunctionTrait<T>::release_array_func(_jni, _j_array, _data);
      _jni->DeleteGlobalRef(_j_array);
      _j_array = nullptr;
      _data = nullptr;
      _size = 0;
    }
  }

  JNIEnv* _jni = nullptr;
  typename TypeToJniType<T>::jni_array_type _j_array = nullptr;
  T* _data = nullptr;
  size_t _size = 0;
};

template <>
class JavaArrayToNative<char, void> final {
 public:
  JavaArrayToNative() = default;
  JavaArrayToNative(JNIEnv* jni, jstring j_str) : _jni(jni) {
    reset(jni, j_str);
  }
  ~JavaArrayToNative() { clear(); }
  void reset(JNIEnv* jni, jstring j_str) {
    if (nullptr == jni || nullptr == j_str) {
      return;
    }
    clear();
    _jni = jni;
    _j_str = static_cast<jstring>(_jni->NewGlobalRef(j_str));
    _c_ptr = _jni->GetStringUTFChars(_j_str, nullptr);
  }

  inline const char* get_data_ptr() const { return _c_ptr; }

  int fill_array(char* ptr, size_t size) {
    size_t size_tmp = strlen(_c_ptr) + 1;
    if (size < size_tmp) {
      return -1;
    }
    memcpy(ptr, _c_ptr, size_tmp);
    return 0;
  }

 private:
  void clear() {
    if (nullptr != _j_str) {
      _jni->ReleaseStringUTFChars(_j_str, _c_ptr);
      _jni->DeleteGlobalRef(_j_str);
      _j_str = nullptr;
      _c_ptr = nullptr;
    }
  }
  JNIEnv* _jni = nullptr;
  jstring _j_str = nullptr;
  const char* _c_ptr = nullptr;
};
//============================ JavaArrayToNative =================================================//
//============================ JavaArrayToNative =================================================//
class JavaObjectMaker {
 public:
  template <class... Args>
  JavaObjectMaker(JNIEnv* jni, const char* signature, ContextTypeRef context,
                  const Args&... args)
      : _jni(jni) {
    jclass clazz = FindClass(_jni, signature);
    RTC_DCHECK(clazz != nullptr);
    std::string func_signature = "(" + function_signature_trait(args...) + ")V";
    jmethodID id = _jni->GetMethodID(clazz, "<init>", func_signature.c_str());
    _obj = _jni->NewObject(clazz, id,
                           FunctionTraitInternal<Args>::get_java_member_func(
                               _jni, args, context)...);
    _jni->DeleteLocalRef(clazz);
  }

  JavaObjectMaker(JNIEnv* jni, const char* signature, ContextTypeRef context)
      : _jni(jni) {
    jclass clazz = FindClass(_jni, signature);
    jmethodID id = _jni->GetMethodID(clazz, "<init>", "()V");
    _obj = _jni->NewObject(clazz, id);
    _jni->DeleteLocalRef(clazz);
  }

  ~JavaObjectMaker() = default;

  jobject get_object() { return _obj; }

 private:
  JNIEnv* _jni = nullptr;
  jobject _obj = nullptr;
};

}  // namespace jni_help
#endif // JNI_COMMON_JNI_HELP_DEFINE_H_
