#include "core/audio/file/audio_file_header.h"

#include "core/audio/file/wav_file_header.h"
namespace core {
std::unique_ptr<AudioFileHeader> AudioFileHeader::CreateWavAudioFileHeader() {
  return std::make_unique<AudioWavFileHeader>();
}
}  // namespace webrtc
