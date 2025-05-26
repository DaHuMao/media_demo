#ifndef CORE_VIDEO_VIDEO_COMMON_FORMAT_DEFINE_H_
#define CORE_VIDEO_VIDEO_COMMON_FORMAT_DEFINE_H_
#include <string>
namespace core {
enum class RawVideoFormat {
  kNone = -1,
  kYUV420P,
  kYUV422P,
  kYUV444P,
  kNV12,
  kRGBA,
  kBGRA,
  kRGB,
  kBGR,
  kNative,
};

enum class VideoEncoderType {
  kNone = -1,
  kH264,
  kH265,
  kVP8,
  kVP9,
  kAV1,
};

struct VideoSize {
  int width = 0;
  int height = 0;
  bool operator==(const VideoSize& other) const {
    return width == other.width && height == other.height;
  }
  std::string ToString() const {
    return std::to_string(width) + "x" + std::to_string(height);
  }
};

struct YUVPlanarVideoStride : public VideoSize {
  int y_stride = 0;
  int u_stride = 0;
  int v_stride = 0;
};

struct UVInterleavedVideoStride : public VideoSize {
  int y_stride = 0;
  int uv_stride = 0;
};

struct AllInterleavedVideoStride : public VideoSize {
  int stride = 0;
};

}  // namespace core
#endif  // CORE_VIDEO_VIDEO_COMMON_FORMAT_DEFINE_H_
