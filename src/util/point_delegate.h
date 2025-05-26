#ifndef UTIL_POINT_DELEGATE_H_
#define UTIL_POINT_DELEGATE_H_
#include <memory>
#include <variant>
namespace util {
/*
 * PointDelegate 用于代理指针的访问，支持裸指针和unique_ptr两种类型
 * 这个用于下面场景：
 * 有一个接口：
 *   class Callback {
 *    public:
 *      void Ondata(const uint8_t* data, size_t size);
 *   };
 * 有两个类：
 *  class AudioProcessor1 {
 *    public:
 *      int Process(const uint8_t* data, size_t size) = 0;
 *      static std::unique_ptr<AudioProcessor1> Create(Callback* callback);
 *  };
 *
 *  class AudioProcessor2 {
 *    public:
 *      int Process(const uint8_t* data, size_t size) = 0;
 *      static std::unique_ptr<AudioProcessor2> Create(Callback* callback);
 *  };
 *
 *  std::unique_ptr<Callback> CreateAudioProcessor1Delegate(AudioProcess1*
 processor1);
 *  std::unique_ptr<Callback> CreateAudioProcessor2Delegate(AudioProcess2*
 processor2);
 *
 *
 *  需求是将数据经过AudioProcessor1处理后，再经过AudioProcessor2处理
 *  正常实现如下：
 *  class Work : public Callback {
 *  public:
 *    Work() {
 *      processor2_ = AudioProcessor2::Create(this);
 *      callback_ = CreateAudioProcessor2Delegate(processor2_.get());
 *      processor1_ = AudioProcessor1::Create(callback_.get());
 *    }
 *  private:
 *    void Ondata(const uint8_t* data, size_t size) override {
 *       ...
      }

 *    std::unique_ptr<AudioProcessor1> processor1_;
 *    std::unique_ptr<AudioProcessor2> processor2_;
 *    std::unique_ptr<Callback> callback_;
 *  };
 *
 *
 从上面的代码可以看出，callback_是一个多余的变量，只是为了将processor2_封装成一个Callback
 *
 *  使用PointDelegate后，可以将callback_去掉，直接将processor2_传递给processor1_
 *  AudioProcess1 跟 AudioProcessor2的create方法改成如下：
 *  std::unique_ptr<AudioProcessor1> Create(PointDelegate<Callback> callback);
 *  std::unique_ptr<AudioProcessor2> Create(PointDelegate<Callback> callback);
 *
 *  class Work : public Callback {
 *    Work() {
 *      processor2_ = AudioProcessor2::Create(this);
 *      processor1_ = AudioProcessor1::Create(
 *                        CreateAudioProcessor2Delegate(processor2_.get()));
 *    }
 *    ...
 *  };
 *  直接将processor2_构造产生的callback托管给processor1_。
 */
template <typename T>
class PointDelegate {
 public:
  PointDelegate(T* ptr) noexcept : ptr_(ptr) {}
  PointDelegate(std::unique_ptr<T>&& ptr) noexcept : ptr_(std::move(ptr)) {}
  PointDelegate(PointDelegate&& other) noexcept : ptr_(std::move(other.ptr_)) {}

  PointDelegate& operator=(PointDelegate&& other) noexcept {
    if (this != &other) {
      ptr_ = std::move(other.ptr_);
    }
    return *this;
  }

  bool operator==(const PointDelegate& other) const noexcept {
    return get() == other.get();
  }

  PointDelegate(const std::unique_ptr<T>&) = delete;
  PointDelegate& operator=(const PointDelegate& other) = delete;
  PointDelegate(const PointDelegate& other) = delete;

  T& operator*() const noexcept { return *get(); }

  T* operator->() const noexcept { return get(); }

  T* get() const noexcept {
    if (auto ptr = std::get_if<T*>(&ptr_)) {
      return *ptr;
    }
    return std::get<std::unique_ptr<T>>(ptr_).get();
  }

  // 检查是否为空
  bool operator!() const noexcept { return get() == nullptr; }

  // 显式转换到 bool
  explicit operator bool() const noexcept { return get() != nullptr; }

 private:
  std::variant<T*, std::unique_ptr<T>> ptr_;
};
}  // namespace util
#endif  // UTIL_POINT_DELEGATE_H_
