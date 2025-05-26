#ifndef WEBRTC_AUDIO_FILE_READER_AUDIO_FILE_HEADER_H_
#define WEBRTC_AUDIO_FILE_READER_AUDIO_FILE_HEADER_H_
#include <functional>

#include "core/audio/audio_format_define.h"
#include "core/audio/audio_io_define.h"
namespace core {
struct AudioFileHeaderInfo {
  AudioFormatInfo audio_format_info;
  size_t audio_frame_size = 0;
  void *extra_data = nullptr;
  size_t extra_data_size = 0;
};

class AudioFileHeader {
 public:
  virtual ~AudioFileHeader() = default;
  virtual bool ReadHeader(AudioFileSource* reader,
                          AudioFileHeaderInfo *audio_file_header_info) = 0;
  virtual bool WriteHeader(
      std::function<size_t(const void *, size_t)> write_func,
      const AudioFormatInfo &audio_format_info, size_t audio_frame_size) = 0;
  static std::unique_ptr<AudioFileHeader> CreateWavAudioFileHeader();
};
}  // namespace core
#endif  // WEBRTC_AUDIO_FILE_READER_AUDIO_FILE_HEADER_H_
