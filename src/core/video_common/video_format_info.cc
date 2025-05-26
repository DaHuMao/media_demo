#include "core/video_common/video_format_info.h"

#include "core/video_common/video_format_util.h"
#include "rtc_base/checks.h"
namespace core {
VideoFormatInfo::VideoFormatInfo(VideoSize video_size, RawVideoFormat format)
    : size_(video_size), format_(format) {
  switch (format_) {
    case RawVideoFormat::kYUV420P:
      strides_ = {size_.width, size_.width / 2, size_.width / 2};
      break;
    case RawVideoFormat::kYUV422P:
      strides_ = {size_.width, size_.width / 2, size_.width / 2};
      break;
    case RawVideoFormat::kYUV444P:
      strides_ = {size_.width, size_.width, size_.width};
      break;
    case RawVideoFormat::kNV12:
      strides_ = {size_.width, size_.width};
      break;
    case RawVideoFormat::kRGBA:
      strides_ = {size_.width * 4};
      break;
    case RawVideoFormat::kRGB:
      strides_ = {size_.width * 3};
      break;
    default:
      RTC_CHECK(false) << "Invalid format";
  }
}
VideoFormatInfo::VideoFormatInfo(VideoSize video_size, RawVideoFormat format,
                                 const std::vector<int>& strides)
    : size_(video_size), strides_(strides), format_(format) {}
size_t VideoFormatInfo::GetAllSizeInByte() const {
  auto length_each_plane = GetLengthEachPlane();
  size_t all_size = 0;
  for (auto length : length_each_plane) {
    all_size += length;
  }
  return all_size;
}

std::vector<int> VideoFormatInfo::GetLengthEachPlane() const {
  switch (format_) {
    case RawVideoFormat::kYUV420P:
      RTC_CHECK_EQ(strides_.size(), 3);
      return {strides_[0] * size_.height, strides_[1] * size_.height / 2,
              strides_[2] * size_.height / 2};
    case RawVideoFormat::kYUV422P:
      RTC_CHECK_EQ(strides_.size(), 3);
      return {strides_[0] * size_.height, strides_[1] * size_.height,
              strides_[2] * size_.height};
    case RawVideoFormat::kYUV444P:
      RTC_CHECK_EQ(strides_.size(), 3);
      return {strides_[0] * size_.height, strides_[1] * size_.height,
              strides_[2] * size_.height};
    case RawVideoFormat::kNV12:
      RTC_CHECK_EQ(strides_.size(), 2);
      return {strides_[0] * size_.height, strides_[1] * size_.height / 2};
    case RawVideoFormat::kRGBA:
      RTC_CHECK_EQ(strides_.size(), 1);
      return {strides_[0] * size_.height};
    case RawVideoFormat::kRGB:
      RTC_CHECK_EQ(strides_.size(), 1);
      return {strides_[0] * size_.height};
    default:
      RTC_CHECK(false) << "Invalid format";
      return {};
  }
}

bool VideoFormatInfo::IsValid() const {
  return format_ != RawVideoFormat::kNone && size_.width > 0 &&
         size_.height > 0;
}

std::string VideoFormatInfo::ToString() const {
  return "VideoFormatInfo: " + size_.ToString() +
         " format: " + core::RawVideoFormatToString(format_);
}
}  // namespace core
