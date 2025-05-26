#include "core/video_common/video_format_util.h"

#include <string>
#include <utility>

#include "util/array_find.h"
namespace core {
constexpr std::pair<RawVideoFormat, const char*> kRawVideoFormatToString[] = {
    {RawVideoFormat::kYUV420P, "YUV420P"},
    {RawVideoFormat::kYUV422P, "YUV422P"},
    {RawVideoFormat::kYUV444P, "YUV444P"},
    {RawVideoFormat::kNV12, "NV12"},
    {RawVideoFormat::kRGBA, "RGBA"},
    {RawVideoFormat::kRGB, "RGB"},
};
bool IsYUVPlanar(RawVideoFormat format) {
  return format == RawVideoFormat::kYUV420P ||
         format == RawVideoFormat::kYUV422P ||
         format == RawVideoFormat::kYUV444P;
}
bool IsUVInterleaved(RawVideoFormat format) {
  return format == RawVideoFormat::kNV12;
}

bool IsYUVInterleaved(RawVideoFormat format) {
  return format == RawVideoFormat::kRGBA || format == RawVideoFormat::kRGB;
}

bool IsRGB(RawVideoFormat format) {
  return format == RawVideoFormat::kRGBA || format == RawVideoFormat::kRGB;
}

int GetPlaneNum(RawVideoFormat format) {
  switch (format) {
    case RawVideoFormat::kYUV420P:
    case RawVideoFormat::kYUV422P:
    case RawVideoFormat::kYUV444P:
      return 3;
    case RawVideoFormat::kNV12:
      return 2;
    case RawVideoFormat::kRGBA:
    case RawVideoFormat::kRGB:
      return 1;
    default:
      return 0;
  }
}

const char* RawVideoFormatToString(RawVideoFormat format) {
  return util::ArrayFind(kRawVideoFormatToString, format, "Unknown");
}
}  // namespace core
