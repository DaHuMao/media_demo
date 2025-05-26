#include "core/audio/audio_format_util.h"

#include "util/array_find.h"
namespace core {
namespace audio_util {
static constexpr char const *kDefaultName = "AudioSampleFormatNone";
static constexpr int kPlanarIndexStart =
    static_cast<int>(AudioSampleFormat::kAudioSampleFormatPcmInt16Planar);

static constexpr std::pair<AudioSampleRate, int> kSampleRateToInt[] = {
    {AudioSampleRate::kNull, 0},
    {AudioSampleRate::kSampleRate8k, 8000},
    {AudioSampleRate::kSampleRate16k, 16000},
    {AudioSampleRate::kSampleRate32k, 32000},
    {AudioSampleRate::kSampleRate44dot1k, 44100},
    {AudioSampleRate::kSampleRate48k, 48000}};

static constexpr std::pair<AudioChannelLayout, int> kChannelLayoutToInt[] = {
    {AudioChannelLayout::kNull, 0},
    {AudioChannelLayout::kMono, 1},
    {AudioChannelLayout::kStereo, 2},
};

static constexpr std::pair<AudioSampleFormat, const char *>
    kAudioSampleFormatName[] = {
        {AudioSampleFormat::kAudioSampleFormatPcmInt16, "kPcmInt16"},
        {AudioSampleFormat::kAudioSampleFormatPcmFloat, "kPcmFloat"},
        {AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar,
         "kPcmFloatPlanar"},
        {AudioSampleFormat::kAudioSampleFormatPcmInt16Planar,
         "kPcmInt16Planar"},
        {AudioSampleFormat::kAudioSampleFormatNone, "kAudioSampleFormatNone"},
};

static constexpr std::pair<int, AudioSampleRate> kIntToSampleRate[] = {
    {8000, AudioSampleRate::kSampleRate8k},
    {16000, AudioSampleRate::kSampleRate16k},
    {32000, AudioSampleRate::kSampleRate32k},
    {44100, AudioSampleRate::kSampleRate44dot1k},
    {48000, AudioSampleRate::kSampleRate48k}};

static constexpr std::pair<int, AudioChannelLayout> kIntToChannelLayout[] = {
    {1, AudioChannelLayout::kMono}, {2, AudioChannelLayout::kStereo}};

static constexpr std::pair<AudioSampleFormat, size_t>
    kAudioSampleFormatSizeMap[] = {
        {AudioSampleFormat::kAudioSampleFormatPcmInt16, sizeof(int16_t)},
        {AudioSampleFormat::kAudioSampleFormatPcmInt16Planar, sizeof(int16_t)},
        {AudioSampleFormat::kAudioSampleFormatPcmFloat, sizeof(float)},
        {AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar, sizeof(float)}};

static constexpr AudioSampleFormat kAudioSampleFormatAllArray[] = {
    AudioSampleFormat::kAudioSampleFormatPcmInt16,
    AudioSampleFormat::kAudioSampleFormatPcmFloat,
    AudioSampleFormat::kAudioSampleFormatPcmInt16Planar,
    AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar};

static constexpr std::pair<AudioSampleFormat, AudioSampleFormat>
    kPlanarToInterleaved[] = {
        {AudioSampleFormat::kAudioSampleFormatPcmInt16Planar,
         AudioSampleFormat::kAudioSampleFormatPcmInt16},
        {AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar,
         AudioSampleFormat::kAudioSampleFormatPcmFloat}};

AudioSampleRate IntToAudioSampleRate(int sample_rate_hz) {
  return util::ArrayFind(kIntToSampleRate, sample_rate_hz,
                         AudioSampleRate::kNull);
}

AudioChannelLayout IntToAudioChannelLayout(int num_channels) {
  return util::ArrayFind(kIntToChannelLayout, num_channels,
                         AudioChannelLayout::kNull);
}

int AudioChannlsLayoutToInt(AudioChannelLayout channels_layout) {
  return util::ArrayFind(kChannelLayoutToInt, channels_layout, 0);
}

int AudioSampleRateToInt(AudioSampleRate audio_sample_rate) {
  return util::ArrayFind(kSampleRateToInt, audio_sample_rate, 0);
}

const char *AudioSampleFormatName(AudioSampleFormat fmt) {
  return util::ArrayFind(kAudioSampleFormatName, fmt, kDefaultName);
}

bool IsPlanar(AudioSampleFormat fmt) {
  return static_cast<int>(fmt) >= kPlanarIndexStart;
}

AudioSampleFormat PlanarToInterleaved(AudioSampleFormat fmt) {
  if (!IsPlanar(fmt)) {
    return fmt;
  }
  return util::ArrayFind(kPlanarToInterleaved, fmt,
                   AudioSampleFormat::kAudioSampleFormatNone);
}

size_t ByteSizePerSample(AudioSampleFormat fmt) {
  return util::ArrayFind(kAudioSampleFormatSizeMap, fmt,
                         static_cast<size_t>(0));
}

bool ValidSampleRate(int sample_rate_hz) {
  return util::ArrayFind(kIntToSampleRate, sample_rate_hz,
                         AudioSampleRate::kNull) != AudioSampleRate::kNull;
}

bool ValidChannel(int channels) {
  return util::ArrayFind(kIntToChannelLayout, channels,
                         AudioChannelLayout::kNull) !=
         AudioChannelLayout::kNull;
}

rtc::ArrayView<const AudioSampleFormat> AudioSampleFormatArray() {
  return rtc::ArrayView<const AudioSampleFormat>(kAudioSampleFormatAllArray);
}

}  // namespace audio_util
}  // namespace core
