#ifndef SRC_LOG_H_
#define SRC_LOG_H_
#include <functional>
#include <sstream>
#include <string>
namespace util {
enum class LogLevel { kDebug, kInfo, kWarnning, kError, kFatal };

struct LogCallBack {
  std::function<void(LogLevel level, const std::string& tag,
      const std::string)> call_back;
  std::function<void()> flush;
};

class LogStream {
public:
  LogStream(LogLevel log_level, const std::string file, int line,
      const std::string& tag);

  ~LogStream();
  template <class T> LogStream &operator<<(const T &val) {
    stream_ << val;
    return *this;
  }

static void SetLogLevel(LogLevel log_level);
static void SetLogCallBack(const LogCallBack &log_call_back);

private:
  std::ostringstream stream_;
  LogLevel log_level_;
  const std::string tag_;
};
}
#define LOGD_TAG(tag) util::LogStream(util::LogLevel::kDebug, __FILE__, __LINE__, tag)
#define LOGI_TAG(tag) util::LogStream(util::LogLevel::kInfo, __FILE__, __LINE__, tag)
#define LOGW_TAG(tag) util::LogStream(util::LogLevel::kWarnning, __FILE__, __LINE__, tag)
#define LOGE_TAG(tag) util::LogStream(util::LogLevel::kError, __FILE__, __LINE__, tag)
#define LOGF_TAG(tag) util::LogStream(util::LogLevel::kFatal, __FILE__, __LINE__, tag)
#endif // SRC_LOG_H_
