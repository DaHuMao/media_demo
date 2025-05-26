#include "core/ffmpeg/ffmpeg_util.h"
#include "util/log.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/log.h"
}
namespace FFmpegUtils {

namespace {
void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl) {
    va_list vl2;
    char line[1024];
    static int print_prefix = 1;

    va_copy(vl2, vl);
    av_log_default_callback(ptr, level, fmt, vl);
    if(level <= AV_LOG_ERROR) {
      av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
      LOGE_TAG("FFmpegLog") << line;
    }
    va_end(vl2);
}
}  // namespace

std::once_flag flag_;

void InitFFmpeg() {
  std::call_once(flag_, [] {
    av_log_set_level(AV_LOG_ERROR);
    av_log_set_callback(ffmpeg_log_callback);
  });
}

std::string GetErrorMessage(int error_code) {
  char err_buf[AV_ERROR_MAX_STRING_SIZE];
  if (av_strerror(error_code, err_buf, sizeof(err_buf)) == 0) {
    return std::string(err_buf);
  } else {
    return "Unknown error";
  }
}

core::FfmpegStatus GetStatus(int ffmpeg_ret) {
  if (ffmpeg_ret == AVERROR(EAGAIN)) {
      return core::FfmpegStatus::kEAgain;
  } else if (ffmpeg_ret == AVERROR_EOF) {
    return core::FfmpegStatus::kEof;
  } else if (ffmpeg_ret < 0) {
    return core::FfmpegStatus::kError;
  }
  return core::FfmpegStatus::kOk;
}

} // namespace FFmpegUtils
