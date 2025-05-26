#include "core/adapter/video_format_converter.h"
#include <utility>
#include "util/array_find.h"
namespace core {
constexpr std::pair<AVPixelFormat, RawVideoFormat> kPixelFormatMap[] = {
    {AV_PIX_FMT_YUV420P, RawVideoFormat::kYUV420P},
    {AV_PIX_FMT_YUV422P, RawVideoFormat::kYUV422P},
    {AV_PIX_FMT_YUV444P, RawVideoFormat::kYUV444P},
    {AV_PIX_FMT_NV12, RawVideoFormat::kNV12},
    {AV_PIX_FMT_RGBA, RawVideoFormat::kRGBA},
    {AV_PIX_FMT_NONE, RawVideoFormat::kNone},
};

RawVideoFormat ToRawVideoFormat(AVPixelFormat format) {
  return util::ArrayFind(kPixelFormatMap, format, RawVideoFormat::kNone);
}
AVPixelFormat ToAVPixelFormat(RawVideoFormat format) {
  return util::ArrayFindKey(kPixelFormatMap, format, AV_PIX_FMT_NONE);
}
} // namespace core
