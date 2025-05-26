#ifndef WEBRTC_COMMON_AUDIO_TEMPLATE_UTIL_H_
#define WEBRTC_COMMON_AUDIO_TEMPLATE_UTIL_H_
#include <functional>
#include <type_traits>
/*
 * Tests whether a class contains a specific member function
 */
#define DefineHasMemberFunctionWithReturnTypeHandle(FunctionName, Function,   \
                                                    ReturnTypeHandle)         \
  template <class... Args>                                                    \
  struct HasMember##FunctionName##Validator {                                 \
    template <                                                                \
        typename T,                                                           \
        typename U = typename std::decay<decltype(std::declval<T>().Function( \
            std::declval<Args>()...))>::type,                                 \
        typename = typename std::enable_if<ReturnTypeHandle<U>::value>::type> \
    static std::true_type Test(int);                                          \
    template <typename>                                                       \
    static std::false_type Test(...);                                         \
  };                                                                          \
  template <class T, class... Args>                                           \
  struct HasFunction##FunctionName##Result                                    \
      : decltype(HasMember##FunctionName##Validator<Args...>::template Test<  \
                 T>(0)) {};

#define DefineHasMemberFunctionWithReturnType(FunctionName, Ret)           \
  template <typename T, typename... Args>               \
  struct HasMemberFunction__##FunctionName {                          \
    private:                                                          \
      /* 这个模板函数在成功匹配到成员函数时启用 */                 \
      template <typename U>                                           \
      static constexpr auto check(U*)                                 \
        -> typename std::is_same<                                      \
              decltype(std::declval<U>().FunctionName(std::declval<Args>()...)), \
              Ret                                                     \
           >::type;                                                   \
                                                                      \
      /* 默认情况下的模板函数 */                                      \
      template <typename>                                             \
      static constexpr std::false_type check(...);                    \
                                                                      \
    public:                                                           \
      /* 判断 check<T>(0) 的返回类型，如果是 true_type 则说明存在该成员函数 */ \
      static constexpr bool value = decltype(check<T>(0))::value;    \
  };

// 判断一个类型是否是std::function
template <typename T>
struct is_std_function : std::false_type {};
template <typename Ret, typename... Args>
struct is_std_function<std::function<Ret(Args...)>> : std::true_type {};

#endif  // WEBRTC_COMMON_AUDIO_TEMPLATE_UTIL_H_
