#ifndef CORE_VIDEO_VIDEO_COMMON_FORMAT_UTIL_H_
#define CORE_VIDEO_VIDEO_COMMON_FORMAT_UTIL_H_
#include "core/video_common/video_format_define.h"
namespace core {
// y u v 分别存储
bool IsYUVPlanar(RawVideoFormat format);
// y单独存储 uv交织
bool IsUVInterleaved(RawVideoFormat format);
// y u v 都是交织
bool IsYUVInterleaved(RawVideoFormat format);
bool IsRGB(RawVideoFormat format);
int GetPlaneNum(RawVideoFormat format);
const char* RawVideoFormatToString(RawVideoFormat format);
}  // namespace core
#endif  // CORE_VIDEO_VIDEO_COMMON_FORMAT_UTIL_H_
