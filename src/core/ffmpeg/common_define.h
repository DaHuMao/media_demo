#ifndef CORE_FFMPEG_COMMON_DEFINE_H_
#define CORE_FFMPEG_COMMON_DEFINE_H_
namespace core {
enum class FfmpegStatus {
  kOk = 0,
  kEof = 1,
  kEAgain = 2,
  kError = -1,
};
} // namespace ffmpeg
#endif // CORE_FFMPEG_COMMON_DEFINE_H_
