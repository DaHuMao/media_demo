#include "core/audio/audio_frame_util.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include "webrtc/rtc_base/checks.h"

#include "common_audio/include/audio_util.h"
namespace core {
namespace audio_util {


template <typename T>
T FindMaxAbsVal(const T *ptr, size_t size) {
  auto compare = [](const T &a, const T &b) {
    return std::abs(a) < std::abs(b);
  };
  return std::abs(*std::max_element(ptr, ptr + size, compare));
}

float GetAudioFrameEnerge(const AudioFrameLiteView &audio_frame) {
  auto fmt = audio_frame.AudioFormat().GetAudioSampleFormat();
  switch (fmt) {
    case AudioSampleFormat::kAudioSampleFormatPcmInt16:
      return FindMaxAbsVal(
                 reinterpret_cast<const int16_t *>(audio_frame.Data()),
                 audio_frame.SampleSize()) *
             1.0 / 32768;
    case AudioSampleFormat::kAudioSampleFormatPcmFloat:
      return FindMaxAbsVal(reinterpret_cast<const float *>(audio_frame.Data()),
                           audio_frame.SampleSize());
    default:
      RTC_DCHECK(false) << "unknow fmt";
  }
  return 0.0f;
}

float GetAudioFrameEnergeDb(const AudioFrameLiteView &audio_frame) {
  auto energy = GetAudioFrameEnerge(audio_frame);
  return 20 * std::log10(energy == 0 ? 0.000001 : energy);
}

template <typename T>
static void Ramp(float start_gain, float end_gain, T *ptr,
                 size_t samples_per_channel, size_t num_channels,
                 bool is_planar) {
  if (start_gain == end_gain) {
    return;
  }
  float increment = (end_gain - start_gain) / samples_per_channel;
  if (is_planar) {
    for (size_t ch = 0; ch < num_channels; ++ch) {
      for (size_t i = 0; i < samples_per_channel; ++i) {
        ptr[samples_per_channel * ch + i] *= start_gain;
      }
      start_gain += increment;
    }
  } else {
    for (size_t i = 0; i < samples_per_channel; ++i) {
      for (size_t ch = 0; ch < num_channels; ++ch) {
        ptr[num_channels * i + ch] *= start_gain;
      }
      start_gain += increment;
    }
  }
}

void Ramp(float start_gain, float end_gain,
          AudioFrameLiteDelegate &audio_frame) {
  auto fmt = audio_frame.AudioFormat().GetAudioSampleFormat();
  switch (fmt) {
    case AudioSampleFormat::kAudioSampleFormatPcmInt16:
      Ramp(start_gain, end_gain,
           reinterpret_cast<int16_t *>(audio_frame.MutableData()),
           audio_frame.FrameSize(),
           audio_frame.AudioFormat().GetChannelsCount(), false);
      break;
    case AudioSampleFormat::kAudioSampleFormatPcmFloat:
      Ramp(start_gain, end_gain,
           reinterpret_cast<float *>(audio_frame.MutableData()),
           audio_frame.FrameSize(),
           audio_frame.AudioFormat().GetChannelsCount(), false);
      break;
    case AudioSampleFormat::kAudioSampleFormatPcmInt16Planar:
      Ramp(start_gain, end_gain,
           reinterpret_cast<int16_t *>(audio_frame.MutableData()),
           audio_frame.FrameSize(),
           audio_frame.AudioFormat().GetChannelsCount(), true);
      break;
    case AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar:
      Ramp(start_gain, end_gain,
           reinterpret_cast<float *>(audio_frame.MutableData()),
           audio_frame.FrameSize(),
           audio_frame.AudioFormat().GetChannelsCount(), true);
      break;
    default:
      RTC_DCHECK(false) << "unknow fmt";
  }
}

template <typename SrcT, typename DestT>
static void PlanarToInterleaved(const SrcT *src, DestT *dest,
                                size_t samples_per_channel, size_t channels,
                                bool is_add_to,
                                std::function<DestT(SrcT)> convert = nullptr) {
  for (size_t ch = 0; ch < channels; ++ch) {
    const SrcT *src_ch = src + ch * samples_per_channel;
    for (size_t i = 0; i < samples_per_channel; ++i) {
      auto src_val = convert ? convert(src_ch[i]) : src_ch[i];
      if (is_add_to) {
        dest[channels * i + ch] += src_val;
      } else {
        dest[channels * i + ch] = src_val;
      }
    }
  }
}

template <typename SrcT, typename DestT>
static void InterleavedToPlanar(const SrcT *src, DestT *dest,
                                size_t samples_per_channel, size_t channels,
                                bool is_add_to,
                                std::function<DestT(SrcT)> convert = nullptr) {
  for (size_t ch = 0; ch < channels; ++ch) {
    DestT *dest_ch = dest + ch * samples_per_channel;
    for (size_t i = 0; i < samples_per_channel; ++i) {
      auto src_val = convert ? convert(src[channels * i + ch])
                             : src[channels * i + ch];
      if (is_add_to) {
        dest_ch[i] += src_val;
      } else {
        dest_ch[i] = src_val;
      }
    }
  }
}

void FloatS16ToAudioFrame(const float *data, size_t data_size,
                          bool float_data_is_planar,
                          AudioFrameLiteDelegate &audio_frame) {
  RTC_DCHECK_EQ(audio_frame.SampleSize(), data_size);
  auto fmt = audio_frame.AudioFormat().GetAudioSampleFormat();
  switch (fmt) {
    case AudioSampleFormat::kAudioSampleFormatPcmInt16:
    case core::AudioSampleFormat::kAudioSampleFormatPcmInt16Planar:
      if (float_data_is_planar == audio_frame.AudioFormat().IsPlanner()) {
        webrtc::FloatS16ToS16(
            data, data_size,
            reinterpret_cast<int16_t *>(audio_frame.MutableData()));
      } else {
        std::function<int16_t(float)> convert = [] (float v) {
          return webrtc::FloatS16ToS16(v);
        };
        if (float_data_is_planar) {
          PlanarToInterleaved(
              data, reinterpret_cast<int16_t *>(audio_frame.MutableData()),
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(),
              /*is_add_to=*/false, convert);
        } else {
          InterleavedToPlanar(
              data, reinterpret_cast<int16_t *>(audio_frame.MutableData()),
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(),
              /*is_add_to=*/false, convert);
        }
      }
      break;
    case AudioSampleFormat::kAudioSampleFormatPcmFloat:
    case core::AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar:
      if (float_data_is_planar == audio_frame.AudioFormat().IsPlanner()) {
        webrtc::FloatS16ToFloat(
            data, data_size,
            reinterpret_cast<float *>(audio_frame.MutableData()));
      } else {
        if (float_data_is_planar) {
          PlanarToInterleaved(
              data, reinterpret_cast<float *>(audio_frame.MutableData()),
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(),
              /*is_add_to=*/false);
        } else {
          InterleavedToPlanar(
              data, reinterpret_cast<float *>(audio_frame.MutableData()),
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(),
              /*is_add_to=*/false);
        }
      }
      break;
    default:
      RTC_DCHECK(false) << "unknow fmt";
  }
}

void AudioFrameToFloatS16(const AudioFrameLiteView &audio_frame, float *data,
                          size_t data_size, bool float_data_is_planar,
                          bool is_add_to) {
  RTC_DCHECK_EQ(data_size, audio_frame.SampleSize());
  auto fmt = audio_frame.AudioFormat().GetAudioSampleFormat();
  switch (fmt) {
    case AudioSampleFormat::kAudioSampleFormatPcmInt16:
    case core::AudioSampleFormat::kAudioSampleFormatPcmInt16Planar:
      if (float_data_is_planar == audio_frame.AudioFormat().IsPlanner()) {
        webrtc::S16ToFloatS16(
            reinterpret_cast<const int16_t *>(audio_frame.Data()),
            audio_frame.SampleSize(), data, is_add_to);
      } else {
        if (float_data_is_planar) {
          InterleavedToPlanar(
              reinterpret_cast<const int16_t *>(audio_frame.Data()), data,
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(), is_add_to);
        } else {
          PlanarToInterleaved(
              reinterpret_cast<const int16_t *>(audio_frame.Data()), data,
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(), is_add_to);
        }
      }
      break;
    case AudioSampleFormat::kAudioSampleFormatPcmFloat:
    case core::AudioSampleFormat::kAudioSampleFormatPcmFloatPlanar:
      if (float_data_is_planar == audio_frame.AudioFormat().IsPlanner()) {
        webrtc::FloatToFloatS16(
            reinterpret_cast<const float *>(audio_frame.Data()),
            audio_frame.SampleSize(), data, is_add_to);
      } else {
        if (float_data_is_planar) {
          InterleavedToPlanar(
              reinterpret_cast<const float *>(audio_frame.Data()), data,
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(), is_add_to);
        } else {
          PlanarToInterleaved(
              reinterpret_cast<const float *>(audio_frame.Data()), data,
              audio_frame.FrameSize(),
              audio_frame.AudioFormat().GetChannelsCount(), is_add_to);
        }
      }
      break;
    default:
      RTC_DCHECK(false) << "unknow fmt";
  }
}

}  // namespace audio_util
}  // namespace core
