#include <memory>

#include "core/audio/file/audio_file_header.h"
#include "core/audio/audio_io_define.h"
#include "core/audio/file/file_io.h"
namespace core {
namespace AudioIOCreator {
std::unique_ptr<AudioFileSource> CreateWavFileSource(
    const std::string &file_name) {
  return std::make_unique<AudioPcmFileSource>(
      file_name, AudioFileHeader::CreateWavAudioFileHeader());
}

std::unique_ptr<AudioRawSink> CreateWavFileWriter(
    const std::string &file_name, const AudioFormatInfo &format_info) {
  return std::make_unique<FileWriterWithHeader>(
      file_name, format_info, AudioFileHeader::CreateWavAudioFileHeader());
}

std::unique_ptr<AudioFileSource> CreatePcmFileSource(
    const std::string &file_name, const AudioFormatInfo &format_info) {
  return std::make_unique<AudioPcmFileSource>(file_name, format_info);
}
}  // namespace AudioIOCreator
}  // namespace core
