#ifndef WEBRTC_AUDIO_ARRAY_FIND_H_
#define WEBRTC_AUDIO_ARRAY_FIND_H_
#include <cstddef>
#include <utility>

#include "rtc_base/checks.h"
namespace util {
template <class K, class V, std::size_t N>
inline V ArrayFind(const std::pair<K, V> (&arr)[N], const K& key, V default_val,
                   bool is_decheck = true) {
  for (size_t i = 0; i < N; ++i) {
    if (key == arr[i].first) {
      return arr[i].second;
    }
  }
  RTC_DCHECK(!is_decheck) << "ArrayFind error not find";
  return default_val;
}

template <class K, class V, std::size_t N>
inline K ArrayFindKey(const std::pair<K, V> (&arr)[N], const V& value,
                      K default_val, bool is_decheck = true) {
  for (size_t i = 0; i < N; ++i) {
    if (value == arr[i].second) {
      return arr[i].first;
    }
  }
  RTC_DCHECK(is_decheck) << "ArrayFind error not find";
  return default_val;
}

}  // namespace util
#endif  // WEBRTC_AUDIO_ARRAY_FIND_H_
