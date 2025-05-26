#ifndef WEBRTC_RTC_BASE_APM_DATA_DISTRIBUTION_DEFINE_H_
#define WEBRTC_RTC_BASE_APM_DATA_DISTRIBUTION_DEFINE_H_
#include "webrtc/audio/utility/apm_data_distribution.h"
namespace webrtc {

namespace  DataDistributionType {
  enum class AudioPipeLineChannel : int {
    kApmIn = 1,
    kApmOut = 1 << 1,
    kAecMic = 1 << 2,
    kAecRef = 1 << 3,
    kAecOut = 1 << 4,
    kNsIn = 1 << 5,
    kNsOut = 1 << 6,
    kPlayout = 1 << 7,
    kRecord = 1 << 8,
    kYfdAgcIn = 1 << 9,
    kYfdAgcOut = 1 << 10,
    kLoopback = 1 << 11,
    kLoopbackForWinRef = 1 << 12,
    kApmInRef = 1 << 13,
    kNone = 1 << 31
  };
  const int kAudioPipeLineChannelIndexMax = static_cast<int>(AudioPipeLineChannel::kApmInRef);
  const char* GetAudioPipeLineChannelName(AudioPipeLineChannel channel_index);

  enum class AudioDataType {
    kPcmS16 = 0,
    kPcmFloatS16,
  };

  struct AudioPipeLineDataFormat {
    int sample_rate;
    int num_channels;
    AudioDataType data_type;
  };

#ifndef DISTRIBUTION_DUMP_HANDLE
#define DISTRIBUTION_DUMP_HANDLE webrtc::DataDistribution< \
  webrtc::DataDistributionType::AudioPipeLineChannel, \
    webrtc::DataDistributionType::AudioPipeLineDataFormat>::GetInstance()
#endif

} // DataDistributionType

} // webrtc
#endif  // WEBRTC_RTC_BASE_APM_DATA_DISTRIBUTION_DEFINE_H_
