#ifndef CORE_CODEC_READER_FFMPEG_H_
#define CORE_CODEC_READER_FFMPEG_H_
#include <functional>
#include <string>

#include "core/audio/audio_format_define.h"
#include "core/ffmpeg/common_define.h"
#include "core/video_common/video_format_info.h"
extern "C" {
#include "libavformat/avformat.h"
}
namespace core {
class ReaderFfmpeg final {
 public:
  struct Config {
    std::string url;
    std::function<bool()> interrupt_callback;
    bool disable_audio = false;
    bool disable_video = false;
  };
  ReaderFfmpeg() = default;
  ~ReaderFfmpeg();
  int InitReader(const Config& config);
  int SeekTo(int64_t ts_ms);
  FfmpegStatus ReadPacket(AVPacket* packet);
  AVStream* GetVideoStream();
  AVStream* GetAudioStream();
  AudioFormatInfo GetAudioFormatInfo();
  VideoFormatInfo GetVideoFormatInfo();
  int GetVideoStreamIndex();
  int GetAudioStreamIndex();
  bool HasVideo();
  bool HasAudio();
  int64_t GetDurationMs();
  int64_t GetAudioDurationMs();
  int64_t GetVideoDurationMs();
  int UnInitReader();

 private:
  struct MediaInfo {
    int64_t duration_ms = 0;
    int audio_stream_index = -1;
    int video_stream_index = -1;
  };
  bool is_init_ = false;
  Config config_;
  MediaInfo media_info_;
  AVFormatContext* format_context_ = nullptr;
};
}  // namespace core
#endif  // CORE_CODEC_READER_FFMPEG_H_
