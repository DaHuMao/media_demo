#include <string>
#include "core/ffmpeg/common_define.h"
namespace FFmpegUtils {
void InitFFmpeg();
std::string GetErrorMessage(int error_code);
core::FfmpegStatus GetStatus(int ffmpeg_ret);
} // namespace FFmpegUtils
