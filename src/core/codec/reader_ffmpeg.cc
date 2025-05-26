#include "core/codec/reader_ffmpeg.h"

#include "core/adapter/audio_format_ffmpeg_convert.h"
#include "core/adapter/video_format_converter.h"
#include "core/audio/audio_format_define.h"
#include "core/audio/audio_format_util.h"
#include "core/ffmpeg/ffmpeg_util.h"
#include "libavutil/avutil.h"
#include "rtc_base/checks.h"
#include "util/log.h"
extern "C" {
#include "libavutil/mathematics.h"
}
const char kLogTag[] = "ReaderFfmpeg";
namespace core {
ReaderFfmpeg::~ReaderFfmpeg() { return; }

int ReaderFfmpeg::InitReader(const Config& config) {
  if (is_init_) {
    return -1;
  }
  if (config_.disable_audio && config_.disable_video) {
    RTC_DCHECK(false) << "disable_audio and disable_video are both true";
    return -1;
  }
  FFmpegUtils::InitFFmpeg();
  config_ = config;
  int ret = -1;
  std::string error_msg;
  do {
    if ((ret = avformat_open_input(&format_context_, config.url.c_str(),
                                   nullptr, nullptr)) < 0) {
      error_msg =
          "av_format_open_input failed, " + FFmpegUtils::GetErrorMessage(ret);
      break;
    }
    if ((ret = avformat_find_stream_info(format_context_, nullptr)) < 0) {
      error_msg = "av_format_find_stream_info failed" +
                  FFmpegUtils::GetErrorMessage(ret);
      break;
    }
    if (!config_.disable_video) {
      media_info_.video_stream_index = av_find_best_stream(
          format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    }
    if (!config_.disable_audio) {
      media_info_.audio_stream_index =
          av_find_best_stream(format_context_, AVMEDIA_TYPE_AUDIO, -1,
                              media_info_.video_stream_index, nullptr, 0);
    }
    if (media_info_.audio_stream_index == -1 &&
        media_info_.video_stream_index == -1) {
      error_msg = "stream not found";
      break;
    }
    media_info_.duration_ms = format_context_->duration / (AV_TIME_BASE / 1000);
  } while (0);
  if (ret != 0) {
    UnInitReader();
    LOGI_TAG(kLogTag) << "InitReader, failed to init. error_msg: " << error_msg
                      << " ret: " << ret;
    return -1;
  }
  is_init_ = true;
  return 0;
}

int ReaderFfmpeg::SeekTo(int64_t ts_ms) {
  if (!is_init_) {
    return -1;
  }
  int64_t ts = av_rescale(ts_ms, AV_TIME_BASE, 1000);
  return avformat_seek_file(format_context_, -1, INT64_MIN, ts, INT64_MAX, 0);
}

FfmpegStatus ReaderFfmpeg::ReadPacket(AVPacket* packet) {
  if (!is_init_) {
    return FfmpegStatus::kError;
  }
  return FFmpegUtils::GetStatus(av_read_frame(format_context_, packet));
}

AVStream* ReaderFfmpeg::GetVideoStream() {
  return media_info_.video_stream_index == -1
             ? nullptr
             : format_context_->streams[media_info_.video_stream_index];
}

AVStream* ReaderFfmpeg::GetAudioStream() {
  return media_info_.audio_stream_index == -1
             ? nullptr
             : format_context_->streams[media_info_.audio_stream_index];
}

int ReaderFfmpeg::GetVideoStreamIndex() {
  return media_info_.video_stream_index;
}

int ReaderFfmpeg::GetAudioStreamIndex() {
  return media_info_.audio_stream_index;
}

bool ReaderFfmpeg::HasVideo() { return media_info_.video_stream_index != -1; }

bool ReaderFfmpeg::HasAudio() { return media_info_.audio_stream_index != -1; }

int64_t ReaderFfmpeg::GetDurationMs() { return media_info_.duration_ms; }

int64_t ReaderFfmpeg::GetAudioDurationMs() {
  auto audio_stream = GetAudioStream();
  return audio_stream == nullptr
             ? 0
             : av_rescale_q(audio_stream->duration, audio_stream->time_base,
                            {1, AV_TIME_BASE});
}

int64_t ReaderFfmpeg::GetVideoDurationMs() {
  auto video_stream = GetVideoStream();
  return video_stream == nullptr
             ? 0
             : av_rescale_q(video_stream->duration, video_stream->time_base,
                            {1, AV_TIME_BASE});
}

int ReaderFfmpeg::UnInitReader() {
  if (!is_init_) {
    return -1;
  }
  return 0;
}

AudioFormatInfo ReaderFfmpeg::GetAudioFormatInfo() {
  auto audio_stream = GetAudioStream();
  if (is_init_ && audio_stream) {
    return AudioFormatInfo(
        core::audio_util::IntToAudioSampleRate(
            audio_stream->codecpar->sample_rate),
        ffmpeg_util::GetChannelLayout(audio_stream->codecpar->channel_layout),
        ffmpeg_util::GetSampleFormat(
            static_cast<AVSampleFormat>(audio_stream->codecpar->format)));
  }
  return kNullAudioFormatInfo;
}

VideoFormatInfo ReaderFfmpeg::GetVideoFormatInfo() {
  auto video_stream = GetVideoStream();
  if (is_init_ && video_stream) {
    return VideoFormatInfo(
        {video_stream->codecpar->width, video_stream->codecpar->height},
        ToRawVideoFormat(
            static_cast<AVPixelFormat>(video_stream->codecpar->format)));
  }
  return VideoFormatInfo({0, 0}, RawVideoFormat::kNone);
}

}  // namespace core
