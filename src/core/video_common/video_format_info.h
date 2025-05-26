#ifndef CORE_VIDEO_VIDEO_COMMON_FORMAT_INFO_H_
#define CORE_VIDEO_VIDEO_COMMON_FORMAT_INFO_H_
#include <cstddef>
#include <string>
#include <vector>

#include "core/video_common/video_format_define.h"
namespace core {
class VideoFormatInfo {
 public:
  VideoFormatInfo(VideoSize video_size, RawVideoFormat format);
  VideoFormatInfo(VideoSize video_size, RawVideoFormat format,
                  const std::vector<int>& strides);
  RawVideoFormat GetFormat() const { return format_; }
  VideoSize GetSize() const { return size_; }
  size_t GetAllSizeInByte() const;
  const std::vector<int>& GetStrides() const { return strides_; }
  std::vector<int> GetLengthEachPlane() const;
  bool IsValid() const;
  std::string ToString() const;

 private:
  VideoSize size_;
  std::vector<int> strides_;
  RawVideoFormat format_ = RawVideoFormat::kNone;
};
}  // namespace core
#endif  // CORE_VIDEO_VIDEO_COMMON_FORMAT_INFO_H_
