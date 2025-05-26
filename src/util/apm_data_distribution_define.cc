#include "webrtc/audio/utility/apm_data_distribution_define.h"
#include <string>
#include <unordered_map>
namespace webrtc {
namespace  DataDistributionType {
  static std::unordered_map<AudioPipeLineChannel, std::string> name_map = {
    {AudioPipeLineChannel::kApmIn, "Apm-In"},
    {AudioPipeLineChannel::kApmOut, "Apm-Out"},
    {AudioPipeLineChannel::kAecMic, "Aec-Mic"},
    {AudioPipeLineChannel::kAecRef, "Aec-Ref"},
    {AudioPipeLineChannel::kAecOut, "Aec-Out"},
    {AudioPipeLineChannel::kNsIn, "Ns-In"},
    {AudioPipeLineChannel::kNsOut, "Ns-Out"},
    {AudioPipeLineChannel::kPlayout, "kPlayout"},
    {AudioPipeLineChannel::kRecord, "kRecord"},
    {AudioPipeLineChannel::kYfdAgcIn, "Yfd-Agc-In"},
    {AudioPipeLineChannel::kYfdAgcOut, "Yfd-Agc-Out"},
    {AudioPipeLineChannel::kLoopback, "Loopback"},
    {AudioPipeLineChannel::kLoopbackForWinRef, "LoopbackForWinRef"},
    {AudioPipeLineChannel::kApmInRef, "ApmInRef"},
  };
  const char* default_name = "";
  const char* GetAudioPipeLineChannelName(AudioPipeLineChannel channel_index) {
    if (name_map.find(channel_index) != name_map.end()) {
      return name_map[channel_index].c_str();
    }
    return default_name;
  }
} // DataDistributionType
} // webrtc
