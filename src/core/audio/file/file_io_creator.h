#ifndef WEBRTC_AUDIO_AUDIO_IO_CREATOR_H_
#define WEBRTC_AUDIO_AUDIO_IO_CREATOR_H_
#include <memory>

#include "core/audio/audio_io_define.h"
namespace core {
namespace AudioIOCreator {
std::unique_ptr<AudioFileSource> CreateWavFileSource(
    const std::string &file_name);
std::unique_ptr<AudioFileSource> CreatePcmFileSource(
    const std::string &file_name, const AudioFormatInfo &format_info);
std::unique_ptr<AudioRawSink> CreateWavFileWriter(
    const std::string &file_name, const AudioFormatInfo &format_info);
std::unique_ptr<AudioRawSink> CreateDumpFileSource(
    const std::string &file_name);
}  // namespace AudioIOCreator
}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_IO_CREATOR_H_
