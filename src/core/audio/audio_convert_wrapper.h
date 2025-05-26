#ifndef CORE_UTIL_AUDIO_CONVERT_WRAPPER_H_
#define CORE_UTIL_AUDIO_CONVERT_WRAPPER_H_
#include "core/audio/audio_converter.h"
#include "core/audio/audio_frame.h"
namespace core {
class AudioConvertWrapper {
 public:
  AudioConvertWrapper(const core::AudioFormatInfo &dst);
  ~AudioConvertWrapper() = default;
  int Convert(const core::AudioFormatInfo &src, const void *data, size_t size);
  int Convert(const core::AudioFrameLiteView &src);
  core::AudioFrameLiteView &GetAudioFrame() { return audio_frame_; }

 private:
  core::AudioFrameLite audio_frame_;
  core::AudioConverterFFmpeg audio_converter_;
};
}  // namespace core
#endif  // CORE_UTIL_AUDIO_CONVERT_WRAPPER_H_
