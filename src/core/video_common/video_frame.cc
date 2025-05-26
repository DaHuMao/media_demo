#include "core/video_common/video_frame.h"

#include "core/video_common/video_format_util.h"
#include "rtc_base/checks.h"
namespace core {
YUVPVideoFrameView::YUVPVideoFrameView(std::vector<uint8_t*> ptr,
                                       YUVPlanarVideoStride size,
                                       RawVideoFormat format,
                                       util::MillisecondsClass time_stamp)
    : size_(size), format_(format), time_stamp_(time_stamp) {
  RTC_CHECK(ptr.size() == 3);
  for (int i = 0; i < 3; i++) {
    this->ptr[i] = ptr[i];
  }
}
YUVPVideoFrameView::YUVPVideoFrameView(uint8_t* ptr, YUVPlanarVideoStride size,
                                       RawVideoFormat format,
                                       util::MillisecondsClass time_stamp)
    : size_(size), format_(format), time_stamp_(time_stamp) {
  RTC_CHECK(IsYUVPlanar(format));
  this->ptr[0] = ptr;
  this->ptr[1] = this->ptr[0] + size.y_stride * size.height;
  switch (format) {
    case RawVideoFormat::kYUV420P:
      this->ptr[2] = this->ptr[1] + size.u_stride * size.height / 2;
      break;
    case RawVideoFormat::kYUV422P:
    case RawVideoFormat::kYUV444P:
      this->ptr[2] = this->ptr[1] + size.u_stride * size.height;
      break;
    default:
      RTC_CHECK(false) << "Invalid format";
  }
}

NV12VideoFrameView::NV12VideoFrameView(uint8_t* ptr,
                                       UVInterleavedVideoStride size,
                                       RawVideoFormat format,
                                       util::MillisecondsClass time_stamp)
    : size_(size), format_(format), time_stamp_(time_stamp) {
  RTC_CHECK(IsUVInterleaved(format));
  this->ptr[0] = ptr;
  this->ptr[1] = this->ptr[0] + size.y_stride * size.height;
}

NV12VideoFrameView::NV12VideoFrameView(std::vector<uint8_t*> ptr,
                                       UVInterleavedVideoStride size,
                                       RawVideoFormat format,
                                       util::MillisecondsClass time_stamp)
    : size_(size), format_(format), time_stamp_(time_stamp) {
  RTC_CHECK(ptr.size() == 2);
  for (int i = 0; i < 2; i++) {
    this->ptr[i] = ptr[i];
  }
}

RGBVideoFrameView::RGBVideoFrameView(uint8_t* ptr, VideoSize size,
                                     RawVideoFormat format)
    : ptr_(ptr), size_(size), format_(format) {
  RTC_CHECK(IsRGB(format));
}

VideoFrameDelegate::VideoFrameDelegate(const std::vector<uint8_t*>& data,
                                       const VideoFormatInfo& info,
                                       util::MillisecondsClass time_stamp)
    : data_(data), info_(info), time_stamp_(time_stamp) {
  RTC_CHECK(data.size() == info.GetStrides().size());
  RTC_CHECK_LE(data.size(), 4);
}

VideoFrame::VideoFrame(const VideoFormatInfo& info) : VideoFrameDelegate(info) {
  auto length_each_plane = info.GetLengthEachPlane();
  size_t all_size = 0;
  for (size_t i = 0; i < length_each_plane.size(); i++) {
    all_size += length_each_plane[i];
  }
  ptr_ = new uint8_t[all_size];
  RTC_CHECK_GE(all_size, info.GetAllSizeInByte());
  data_.resize(length_each_plane.size());
  uint8_t* ptr = ptr_;
  for (size_t i = 0; i < data_.size(); i++) {
    data_[i] = ptr;
    ptr += length_each_plane[i];
  }
}

VideoFrame::~VideoFrame() {
  if (ptr_) {
    delete[] ptr_;
  }
}

}  // namespace core
