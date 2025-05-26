#ifndef BIZ_AUDIO_INCLUDE_CALLBACK_REGISTER_TEMPLATE_H_
#define BIZ_AUDIO_INCLUDE_CALLBACK_REGISTER_TEMPLATE_H_
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

#include "rtc_base/checks.h"
#include "util/template_util.h"
#include "util/point_delegate.h"
namespace util {
template <typename T>
class CallbackRegisterTemplate {
 public:
  using CallbackType = std::conditional_t<is_std_function<T>::value, T, T*>;
  CallbackRegisterTemplate() = default;
  ~CallbackRegisterTemplate() = default;
  void RegisterCallback(CallbackType callback) {
    auto it = std::find(callback_list_.begin(), callback_list_.end(), callback);
    if (it != callback_list_.end()) {
      return;
    }
    for (size_t i = 0; i < callback_list_.size(); i++) {
      if (callback_list_[i] == nullptr) {
        callback_list_[i] = std::move(callback);
        return;
      }
    }
    callback_list_.emplace_back(std::move(callback));
  }
  void UnRegisterCallback(CallbackType callback) {
    auto it = std::find(callback_list_.begin(), callback_list_.end(), callback);
    if (it != callback_list_.end()) {
      *it = nullptr;
    }
  }

  const std::vector<CallbackType>& GetCallbackList() const {
    return callback_list_;
  }
 protected:
  std::vector<CallbackType> callback_list_;
};

DefineHasMemberFunctionWithReturnType(Reset, void)
DefineHasMemberFunctionWithReturnType(Validate, bool)
constexpr int kMaxObjectSize = std::numeric_limits<int>::max() / 2;
template <typename ObjType>
class ObjectRegisterTemplate {
  static_assert(HasMemberFunction__Reset<ObjType>::value,
                "ObjType must have <void Reset()> method");
  static_assert(HasMemberFunction__Validate<ObjType>::value,
                "ObjType must have <bool Validate()> method");

 public:
  ObjectRegisterTemplate() {
    std::random_device rd;
    std::mt19937 gen(rd());
    int max_random = std::numeric_limits<int>::max() - kMaxObjectSize - 1;
    std::uniform_int_distribution<> dis(0, max_random);
    rand_id_ = dis(gen);
  }
  ~ObjectRegisterTemplate() = default;
  template <typename CanEqualObjType, typename... Args>
  int RegisterObject(CanEqualObjType&& key, Args&&... other_args) {
    int index = -1;
    for (int i = 0; i < object_list_.size(); i++) {
      if (object_list_[i] == key) {
        RTC_DCHECK(false) << "Object already registered";
        return -1;
      } else if (!object_list_[i].Validate()) {
        index = i;
      }
    }
    if (index != -1) {
      object_list_[index] = ObjType(key, std::forward<Args>(other_args)...);
    } else {
      object_list_.emplace_back(key, std::forward<Args>(other_args)...);
      index = object_list_.size() - 1;
    }
    return index + rand_id_;
  }

  template <typename CanEqualObjType>
  int UnRegisterObject(const CanEqualObjType& object) {
    int index = FindObject(object);
    if (index < 0 || index >= static_cast<int>(object_list_.size())) {
      RTC_DCHECK(false) << "Object not found";
      return -1;
    }
    object_list_[index].Reset();
    return 0;
  }

  int UnRegisterObject(int id) {
    int index = id - rand_id_;
    if (index < 0 || index >= static_cast<int>(object_list_.size())) {
      RTC_DCHECK(false) << "Object not found";
      return -1;
    }
    if (object_list_[index].Validate()) {
      object_list_[index].Reset();
    }
    return 0;
  }

 protected:
  ObjType* GetObject(int id) {
    int index = id - rand_id_;
    if (index < 0 || index >= static_cast<int>(object_list_.size())) {
      return nullptr;
    }
    return &object_list_[index];
  }

  template <typename CanEqualObjType>
  int FindObject(const CanEqualObjType& object) {
    for (size_t i = 0; i < object_list_.size(); i++) {
      if (object_list_[i] == object) {
        return i;
      }
    }
    return -1;
  }
  bool Empty() const { return object_list_.empty(); }
  std::vector<ObjType> object_list_;

 private:
  int rand_id_;
};

}  // namespace yuanli
#endif  // BIZ_AUDIO_INCLUDE_CALLBACK_REGISTER_TEMPLATE_H_
