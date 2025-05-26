#ifndef WEBRTC_AUDIO_TIME_TO_CLASS_H_
#define WEBRTC_AUDIO_TIME_TO_CLASS_H_
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ostream>
#include <type_traits>

namespace util {
template <typename T>
class BaseTimeData {
 public:
  constexpr explicit BaseTimeData() : data_(0) {}
  constexpr explicit BaseTimeData(T data) : data_(data) {}
  constexpr bool operator>(const BaseTimeData<T>& data) const {
    return data_ > data.data_;
  }
  constexpr bool operator>=(const BaseTimeData<T>& data) const {
    return data_ >= data.data_;
  }
  constexpr bool operator<(const BaseTimeData<T>& data) const {
    return data_ < data.data_;
  }
  constexpr bool operator<=(const BaseTimeData<T>& data) const {
    return data_ <= data.data_;
  }
  constexpr bool operator==(const BaseTimeData<T>& data) const {
    return data_ == data.data_;
  }
  constexpr bool operator!=(const BaseTimeData<T>& data) const {
    return data_ != data.data_;
  }

  constexpr void operator+=(const BaseTimeData<T>& data) {
    data_ += data.data_;
  }
  constexpr void operator-=(const BaseTimeData<T>& data) {
    data_ -= data.data_;
  }
  constexpr void operator*=(T data) { data_ *= data; }
  constexpr void operator/=(T data) { data_ /= data; }

  constexpr T BaseData() const { return data_; }

 protected:
  T data_;
};

#define DEFINE_TIME_CLASS(name, type, factor)                               \
  constexpr explicit name() : BaseTimeData<type>(0) {}                      \
  constexpr explicit name(type data) : BaseTimeData<type>(data * factor) {} \
  constexpr name operator+(name data) const {                               \
    return name(this->data_ + data.Value());                                \
  }                                                                         \
  constexpr name operator-(name data) const {                               \
    return name(this->data_ - data.Value());                                \
  }                                                                         \
  constexpr name operator-() const { return name(-this->data_); }           \
  constexpr name operator*(type data) const {                               \
    return name(this->data_ * data);                                        \
  }                                                                         \
  constexpr name operator/(type data) const {                               \
    return name(this->data_ / data);                                        \
  }                                                                         \
  constexpr float operator/(const BaseTimeData<T>& data) const {            \
    return 1.0f * this->data_ / data.BaseData();                            \
  }

template <typename T, typename K = void>
struct CheckData {
  static constexpr void Check(T data, size_t factor) {}
};

template <typename T>
struct CheckData<T, typename std::enable_if<!std::is_same<T, bool>::value &&
                                            std::is_integral<T>::value>::type> {
  static constexpr void Check(T data, size_t factor) {
    if (data % factor != 0) {
      std::abort();
    }
  }
};

template <typename T, size_t factor>
class TimeBaseClass final : public BaseTimeData<T> {
 public:
  constexpr explicit TimeBaseClass(int data)
      : BaseTimeData<T>(static_cast<T>(data * factor)) {}
  template <size_t factor2>
  constexpr TimeBaseClass(const TimeBaseClass<T, factor2>& data)
      : BaseTimeData<T>(data) {}
  constexpr T Value() const {
    CheckData<T>::Check(this->data_, factor);
    return this->data_ / factor;
  }
  DEFINE_TIME_CLASS(TimeBaseClass, T, factor)
};

template <typename T, size_t factor>
std::ostream& operator<<(std::ostream& os,
                         const TimeBaseClass<T, factor>& obj) {
  os << obj.BaseData();
  return os;
}

typedef TimeBaseClass<int64_t, 1> MillisecondsClass;
typedef TimeBaseClass<int64_t, 1000> SecondsClass;
typedef TimeBaseClass<int64_t, 1000 * 60> MinutesClass;
typedef TimeBaseClass<int64_t, 1000 * 60 * 60> HoursClass;

inline MillisecondsClass TimeNow() {
  return MillisecondsClass(static_cast<int64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count()));
}
}  // namespace util
constexpr util::MillisecondsClass operator""_ms(unsigned long long a) {
  return util::MillisecondsClass(static_cast<int64_t>(a));
}

constexpr util::SecondsClass operator""_sec(unsigned long long a) {
  return util::SecondsClass(static_cast<int64_t>(a));
}

constexpr util::MinutesClass operator""_min(unsigned long long a) {
  return util::MinutesClass(static_cast<int64_t>(a));
}

constexpr util::HoursClass operator""_hour(unsigned long long a) {
  return util::HoursClass(static_cast<int64_t>(a));
}
#endif  // WEBRTC_AUDIO_TIME_TO_CLASS_H_
