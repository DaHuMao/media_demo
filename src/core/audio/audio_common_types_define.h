#ifndef WEBRTC_AUDIO_AUDIO_COMMON_TYPES_DEFINE_H_
#define WEBRTC_AUDIO_AUDIO_COMMON_TYPES_DEFINE_H_
#include <cstdint>
namespace core {
enum VadStatus {
  kVadStatusNone = -1,
  kVadStatusPassive,
  kVadStatusActive,
};

enum class AudioSampleRate : uint16_t {
  kNull = 0,
  kSampleRate8k = 8000,
  kSampleRate16k = 16000,
  kSampleRate32k = 32000,
  kSampleRate44dot1k = 44100,
  kSampleRate48k = 48000,
};

enum class AudioChannelLayout : uint8_t {
  kNull = 0,
  kMono,
  kStereo,
};

enum class AudioSampleFormat : int {
  kAudioSampleFormatNone = -1,
  kAudioSampleFormatPcmInt16 = 0,
  kAudioSampleFormatPcmFloat = 1,

  // all planner >= 101
  kAudioSampleFormatPcmInt16Planar = 101,
  kAudioSampleFormatPcmFloatPlanar = 102,
};

}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_COMMON_TYPES_DEFINE_H_
