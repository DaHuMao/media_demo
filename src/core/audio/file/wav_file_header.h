#ifndef WEBRTC_AUDIO_FILE_READER_WAV_FILE_HEADER_H_
#define WEBRTC_AUDIO_FILE_READER_WAV_FILE_HEADER_H_
#include "core/audio/file/audio_file_header.h"
namespace core {
class AudioWavFileHeader final : public AudioFileHeader {
 public:
  AudioWavFileHeader() = default;
  ~AudioWavFileHeader() override = default;
  bool ReadHeader(AudioFileSource* read,
                  AudioFileHeaderInfo* audio_file_header_info) override;
  bool WriteHeader(std::function<size_t(const void*, size_t)> write_func,
                   const AudioFormatInfo& audio_format_info,
                   size_t audio_frame_size) override;
};
}  // namespace webrtc
#endif  // WEBRTC_AUDIO_FILE_READER_WAV_FILE_HEADER_H_
