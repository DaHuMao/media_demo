#ifndef CORE_VIDEO_VIDEO_COMMON_FRAME_H_
#define CORE_VIDEO_VIDEO_COMMON_FRAME_H_
#include <cstdint>
#include <vector>

#include "core/video_common/video_format_define.h"
#include "core/video_common/video_format_info.h"
#include "util/time_to_class.h"
namespace core {
class YUVPVideoFrameView {
 public:
  YUVPVideoFrameView(std::vector<uint8_t*> ptr, YUVPlanarVideoStride size,
                     RawVideoFormat format, util::MillisecondsClass time_stamp);
  YUVPVideoFrameView(uint8_t* ptr, YUVPlanarVideoStride size,
                     RawVideoFormat format, util::MillisecondsClass time_stamp);
  uint8_t* GetY() const { return ptr[0]; }
  uint8_t* GetU() const { return ptr[1]; }
  uint8_t* GetV() const { return ptr[2]; }
  util::MillisecondsClass GetTimeStamp() const { return time_stamp_; }
  YUVPlanarVideoStride GetSize() const { return size_; }
  RawVideoFormat GetFormat() const { return format_; }

 protected:
  uint8_t* ptr[3] = {nullptr};
  YUVPlanarVideoStride size_;
  // 必须为yuv planar格式
  RawVideoFormat format_;
  util::MillisecondsClass time_stamp_;
};

class NV12VideoFrameView {
 public:
  NV12VideoFrameView(uint8_t* ptr, UVInterleavedVideoStride size,
                     RawVideoFormat format, util::MillisecondsClass time_stamp);
  NV12VideoFrameView(std::vector<uint8_t*> ptr, UVInterleavedVideoStride size,
                     RawVideoFormat format, util::MillisecondsClass time_stamp);
  uint8_t* GetY() const { return ptr[0]; }
  uint8_t* GetUV() const { return ptr[1]; }
  util::MillisecondsClass GetTimeStamp() const { return time_stamp_; }
  UVInterleavedVideoStride GetSize() const { return size_; }
  RawVideoFormat GetFormat() const { return format_; }

 protected:
  uint8_t* ptr[2] = {nullptr};
  UVInterleavedVideoStride size_;
  // 必须为nv12格式
  RawVideoFormat format_;
  util::MillisecondsClass time_stamp_;
};

class RGBVideoFrameView {
 public:
  RGBVideoFrameView(uint8_t* ptr, VideoSize size, RawVideoFormat format);
  uint8_t* GetPtr() const { return ptr_; }
  util::MillisecondsClass GetTimeStamp() const { return time_stamp_; }
  VideoSize GetSize() const { return size_; }
  RawVideoFormat GetFormat() const { return format_; }

 protected:
  uint8_t* ptr_ = nullptr;
  VideoSize size_;
  // 必须为rgb格式
  RawVideoFormat format_;
  util::MillisecondsClass time_stamp_;
};

class VideoFrameDelegate {
 public:
  VideoFrameDelegate(const std::vector<uint8_t*>& data,
                     const VideoFormatInfo& info,
                     util::MillisecondsClass time_stamp);
  // dim of <GetData> and <GetStride> must be equal
  const std::vector<uint8_t*>& GetData() const { return data_; }
  VideoFormatInfo GetFormat() const { return info_; }
  util::MillisecondsClass GetTimeStamp() const { return time_stamp_; }

 protected:
  VideoFrameDelegate(const VideoFormatInfo& info) : info_(info) {}
  std::vector<uint8_t*> data_;
  VideoFormatInfo info_;
  util::MillisecondsClass time_stamp_;
};

class VideoFrame : public VideoFrameDelegate {
 public:
  VideoFrame(const VideoFormatInfo& info);
  ~VideoFrame();
  std::vector<uint8_t*> GetData() { return data_; }
  uint8_t* GetOriginalPoint() const { return ptr_; }
 private:
  uint8_t* ptr_ = nullptr;
};

}  // namespace core
#endif  // CORE_VIDEO_VIDEO_COMMON_FRAME_H_
