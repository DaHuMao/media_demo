#ifndef CORE_AUDIO_AUDIO_MIXER_H_
#define CORE_AUDIO_AUDIO_MIXER_H_
#include <functional>

#include "core/audio/audio_io_define.h"
namespace core {
class AudioMixer {
 public:
  virtual ~AudioMixer() = default;
  // 默认读取长度为audio_frame.CapacityInByte()，如果读取失败返回0
  virtual size_t Read(void* data,
      size_t read_size, const AudioFormatInfo& info) = 0;
  virtual int32_t AddSource(core::AudioRawSource* source,
                            std::function<void()> on_remove) = 0;
  virtual int32_t RemoveSource(core::AudioRawSource* source) = 0;
  virtual size_t SourceCount() = 0;
  static std::unique_ptr<AudioMixer> Create(
      std::function<void()> on_all_remove);
};
}  // namespace core
#endif  // CORE_AUDIO_AUDIO_MIXER_H_
