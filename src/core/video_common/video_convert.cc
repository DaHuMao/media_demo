#include "core/video_common/video_convert.h"

#include "libyuv/convert_argb.h"
#include "rtc_base/checks.h"
namespace core {

#include <vector>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

// FFmpegVideoConverter 类封装 YUV 到 RGB 的转换
AVFrame* srcFrame = av_frame_alloc();
AVFrame* dstFrame = av_frame_alloc();
class FFmpegVideoConverter {
 public:
  static int ToRGB(const VideoFrameDelegate& frame,
                   VideoFrameDelegate& out_frame) {
    auto yuv_data = frame.GetData();
    auto yuv_size = frame.GetFormat().GetSize();
    auto yuv_strides = frame.GetFormat().GetStrides();

    // 创建源图像 AVFrame
    srcFrame->format = AV_PIX_FMT_YUV420P;  // 默认设置为 YUV420P 格式
    srcFrame->width = yuv_size.width;
    srcFrame->height = yuv_size.height;

    for (int i = 0; i < 3; ++i) {
      srcFrame->linesize[i] = yuv_strides[i];
    }
    for (int i = 0; i < 3; ++i) {
      srcFrame->data[i] = yuv_data[i];
    }

    // 创建目标图像 AVFrame
    dstFrame->format = AV_PIX_FMT_RGB24;
    dstFrame->width = yuv_size.width;
    dstFrame->height = yuv_size.height;

    dstFrame->linesize[0] = yuv_size.width * 3;
    dstFrame->data[0] = out_frame.GetData()[0];

    // 创建转换上下文
    SwsContext* swsCtx =
        sws_getContext(yuv_size.width, yuv_size.height, AV_PIX_FMT_YUV420P,
                       yuv_size.width, yuv_size.height, AV_PIX_FMT_RGB24,
                       SWS_BILINEAR, nullptr, nullptr, nullptr);

    // 执行转换
    sws_scale(swsCtx, srcFrame->data, srcFrame->linesize, 0, yuv_size.height,
              dstFrame->data, dstFrame->linesize);

    // 释放资源
    sws_freeContext(swsCtx);

    return 0;
  }
};

int ToRGB(const VideoFrameDelegate& frame, VideoFrameDelegate& out_frame) {
  switch (frame.GetFormat().GetFormat()) {
    case RawVideoFormat::kYUV420P: {
      auto yuv_data = frame.GetData();
      auto yuv_size = frame.GetFormat().GetSize();
      auto yuv_strides = frame.GetFormat().GetStrides();
      libyuv::I420ToRGB24(yuv_data[0], yuv_strides[0], yuv_data[1],
                          yuv_strides[1], yuv_data[2], yuv_strides[2],
                          out_frame.GetData()[0],
                          out_frame.GetFormat().GetSize().width * 3,
                          yuv_size.width, yuv_size.height);
      break;
    }
    case RawVideoFormat::kYUV422P: {
      auto yuv_data = frame.GetData();
      auto yuv_size = frame.GetFormat().GetSize();
      auto yuv_strides = frame.GetFormat().GetStrides();
      libyuv::I422ToRGB24(yuv_data[0], yuv_strides[0], yuv_data[1],
                          yuv_strides[1], yuv_data[2], yuv_strides[2],
                          out_frame.GetData()[0],
                          out_frame.GetFormat().GetSize().width * 3,
                          yuv_size.width, yuv_size.height);
      break;
    }
    case RawVideoFormat::kYUV444P: {
      auto yuv_data = frame.GetData();
      auto yuv_size = frame.GetFormat().GetSize();
      auto yuv_strides = frame.GetFormat().GetStrides();
      libyuv::I444ToRGB24(yuv_data[0], yuv_strides[0], yuv_data[1],
                          yuv_strides[1], yuv_data[2], yuv_strides[2],
                          out_frame.GetData()[0],
                          out_frame.GetFormat().GetSize().width * 3,
                          yuv_size.width, yuv_size.height);
      break;
    }
    case RawVideoFormat::kNV12: {
      auto yuv_data = frame.GetData();
      auto yuv_size = frame.GetFormat().GetSize();
      auto yuv_strides = frame.GetFormat().GetStrides();
      libyuv::NV12ToRGB24(yuv_data[0], yuv_strides[0], yuv_data[1],
                          yuv_strides[1], out_frame.GetData()[0],
                          out_frame.GetFormat().GetSize().width * 3,
                          yuv_size.width, yuv_size.height);
      break;
    }
    default:
      return -1;
  }
  return 0;
}

int VideoConvert::Convert(const VideoFrameDelegate& frame,
                          VideoFrameDelegate& out_frame) {
  switch (out_frame.GetFormat().GetFormat()) {
    case RawVideoFormat::kRGB:
      // return FFmpegVideoConverter::ToRGB(frame, out_frame);
      return ToRGB(frame, out_frame);
    default:
      RTC_CHECK(false) << "Invalid format";
      return -1;
  }
}
}  // namespace core
