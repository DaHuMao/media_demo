#ifndef CORE_VIDEO_VIDEO_FORMAT_CONVERTER_H_
#define CORE_VIDEO_VIDEO_FORMAT_CONVERTER_H_
#include "core/video_common/video_format_define.h"
#include "libavutil/pixfmt.h"
namespace core {
RawVideoFormat ToRawVideoFormat(AVPixelFormat format);
AVPixelFormat ToAVPixelFormat(RawVideoFormat format);
}  // namespace core
#endif  // CORE_VIDEO_VIDEO_FORMAT_CONVERTER_H_
