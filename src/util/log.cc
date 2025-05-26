#include "util/log.h"
#include <cstdlib>
#include <iostream>
#if defined(MEDIA_ANDROID)
#include <android/log.h>
#endif
namespace util {
#if !defined(MEDIA_ANDROID)
static std::string LogLevelToString(LogLevel log_level) {
  switch (log_level) {
    case LogLevel::kDebug:
      return "DEBUG";
    case LogLevel::kInfo:
      return "INFO";
    case LogLevel::kWarnning:
      return "WARNNING";
    case LogLevel::kError:
      return "ERROR";
    case LogLevel::kFatal:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}
#endif
static void default_log_callback(LogLevel level, const std::string &tag,
    const std::string &log) {
#if defined(MEDIA_ANDROID)
  switch(level) {
    case LogLevel::kDebug:
      __android_log_print(ANDROID_LOG_DEBUG, tag.c_str(), "%s", log.c_str());
      break;
    case LogLevel::kInfo:
      __android_log_print(ANDROID_LOG_INFO, tag.c_str(), "%s", log.c_str());
      break;
    case LogLevel::kWarnning:
      __android_log_print(ANDROID_LOG_WARN, tag.c_str(), "%s", log.c_str());
      break;
    case LogLevel::kError:
      __android_log_print(ANDROID_LOG_ERROR, tag.c_str(), "%s", log.c_str());
      break;
    case LogLevel::kFatal:
      __android_log_print(ANDROID_LOG_FATAL, tag.c_str(), "%s", log.c_str());
      break;
    default:
      __android_log_print(ANDROID_LOG_ERROR, tag.c_str(), "%s", log.c_str());
      break;
  }
#else
  std::cerr << LogLevelToString(level) << " " << tag << ": " << log;
#endif
}

void default_flush_callback() {
  std::cerr.flush();
}

static LogLevel g_log_level = LogLevel::kDebug;
static LogCallBack g_log_call_back = {default_log_callback,
  default_flush_callback};


void LogStream::SetLogLevel(LogLevel log_level) {
  g_log_level = log_level;
}

void LogStream::SetLogCallBack(const LogCallBack &log_call_back) {
  g_log_call_back = log_call_back;
}

LogStream::LogStream(LogLevel log_level, const std::string file, int line,
      const std::string& tag)
  : log_level_(log_level),
    tag_(tag) {
  if (log_level_ < g_log_level) {
    return;
  }
  if (log_level == LogLevel::kFatal || log_level == LogLevel::kError) {
    stream_ << " file " << file << " line " << line << " ";
  }
}

LogStream::~LogStream() {
  if (log_level_ < g_log_level) {
    return;
  }
  stream_ << std::endl;
  if (g_log_call_back.call_back) {
    g_log_call_back.call_back(log_level_, tag_, stream_.str());
  }
  if (log_level_ == LogLevel::kFatal) {
    if (g_log_call_back.flush) {
      g_log_call_back.flush();
    }
    abort();
  }
}
} // namespace util
