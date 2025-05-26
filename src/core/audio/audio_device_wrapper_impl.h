
/**
 * audio_device_node_impl.h, April 2021.
 * create by zhangtongxiao
 * Copyright 2020 fenbi.com. All rights reserved.
 * FENBI.COM PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef AUDIO_ENGINE_AUDIO_ENGINE_CORE_INCLUDE_AUDIO_DEVICE_NODE_IMPL_H_
#define AUDIO_ENGINE_AUDIO_ENGINE_CORE_INCLUDE_AUDIO_DEVICE_NODE_IMPL_H_
#include <shared_mutex>
#include <vector>

#include "webrtc/api/audio/audio_device.h"
#include "webrtc/api/audio/audio_device_defines.h"
#include "webrtc/api/task_queue/task_queue_factory.h"
#include "webrtc/rtc_base/thread.h"

#include "core/audio/audio_device.h"
#include "core/audio/audio_io_define.h"
#include "core/audio/audio_mixer.h"
#include "util/callback_register_template.h"
namespace rtc {
class Thread;
}
namespace core {
class AudioDeviceWrapperImpl final
    : public webrtc::AudioTransport,
      public AudioDeviceWrapper,
      public util::CallbackRegisterTemplate<RecordingEventCallBack>,
      public util::CallbackRegisterTemplate<PlayoutEventCallback>,
      public util::CallbackRegisterTemplate<core::AudioRawSink>,
      public util::CallbackRegisterTemplate<core::AudioRawSource> {
 public:
  AudioDeviceWrapperImpl();
  ~AudioDeviceWrapperImpl() override;

  // -------------------- AudioDeviceWrapper
  int32_t EnableBuiltInAec(bool enableaec) override;
  int32_t EnableLowLatancy(bool enable) override;
  const core::AudioFormatInfo& GetDefaultRecordAudioFormatInfo() override;
  const core::AudioFormatInfo& GetDefaultPlayoutAudioFormatInfo() override;
  int32_t AddRecordSink(core::AudioRawSink* sink) override;
  int32_t RemoveRecordSink(core::AudioRawSink* sink) override;
  int32_t AddPlayoutSink(core::AudioRawSink* sink) override;
  int32_t RemovePlayoutSink(core::AudioRawSink* sink) override;
  int32_t AddPlayoutSource(core::AudioRawSource* source) override;
  int32_t RemovePlayoutSource(core::AudioRawSource* source) override;
#if defined(WEBRTC_MAC_ONLY) || defined(WEBRTC_WIN)
  std::vector<std::pair<std::string, std::string>> GetRecordDeviceList()
      override;
  std::vector<std::pair<std::string, std::string>> GetPlayoutDeviceList()
      override;
  int32_t SetRecordingDevice(const char* guid) override;
  int32_t SetPlayoutDevice(const char* guid) override;
#endif  // (defined(WEBRTC_MAC) && defined(WEBRTC_IOS)) || defined(WEBRTC_WIN)
  int AddRecordingEventSubscriber(RecordingEventCallBack* callback) override;
  int RemoveRecordingEventSubscriber(RecordingEventCallBack* callback) override;
  int AddPlayoutEventSubscriber(PlayoutEventCallback* callback) override;
  int RemovePlayoutEventSubscriber(PlayoutEventCallback* callback) override;
  util::MillisecondsClass GetPlayoutDelayMs() override;
  // -------------------- AudioDeviceWrapper

  // --------------------- webrtc::AudioTransport
  int32_t RecordedDataIsAvailable(
      const void* audioSamples, const size_t nSamples,
      const size_t nBytesPerSample, const size_t nChannels,
      const uint32_t samplesPerSec, const uint32_t totalDelayMS,
      const int32_t clockDrift, const uint32_t currentMicLevel,
      const bool keyPressed, uint32_t& newMicLevel) override;
  int32_t NeedMorePlayData(const size_t nSamples, const size_t nBytesPerSample,
                           const size_t nChannels, const uint32_t samplesPerSec,
                           void* audioSamples, size_t& nSamplesOut,
                           int64_t* elapsed_time_ms,
                           int64_t* ntp_time_ms) override;
  void PullRenderData(int bits_per_sample, int sample_rate,
                      size_t number_of_channels, size_t number_of_frames,
                      void* audio_data, int64_t* elapsed_time_ms,
                      int64_t* ntp_time_ms) override {}
  // --------------------- webrtc::AudioTransport

  int32_t GetRecordAudioSource() const;
  int32_t SetRecordAudioSource(int32_t audioSource);
  int32_t GetPlayoutAudioSource() const;
  int32_t SetPlayoutAudioSource(int32_t audioSource);
  int StartRecordingAsync();
  int StartPlayoutAsync();
  int StopRecordingAsync(int delay_ms);
  int StopPlayoutAsync(int delay_ms);
  int Terminate();

 private:
  void ReCreateAudioDeviceIfNeed(
      webrtc::AudioDeviceModule::AudioLayer audio_layer);
  void CheckIfStartRecording();
  void CheckIfStopRecording();
  void CheckIfStartPlayout();
  void CheckIfStopPlayout();

 private:
#if defined(WEBRTC_MAC_ONLY) || defined(WEBRTC_WIN)
  std::vector<std::pair<std::string, std::string>> GetRecordDeviceListInternal();
  std::vector<std::pair<std::string, std::string>> GetPlayoutDeviceListInternal();
  int record_index_ = 0;
  int playout_index_ = 0;
#endif
  bool enable_buildin_aec_ = false;
  bool first_record_frame_ = true;
  bool first_playout_frame_ = true;
  webrtc::AudioDeviceModule::AudioLayer audio_layer_ =
      webrtc::AudioDeviceModule::kPlatformDefaultAudio;
  rtc::scoped_refptr<webrtc::AudioDeviceModule> adm_;
  std::unique_ptr<rtc::Thread> signal_thread_ = nullptr;
  std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_ = nullptr;
  util::CallbackRegisterTemplate<core::AudioRawSink> playout_sink_;
  std::shared_mutex shared_mutex_;
  std::unique_ptr<AudioMixer> audio_mixer_;
  AudioRawSource* source_ = nullptr;
};
}  // namespace core
#endif  // AUDIO_ENGINE_AUDIO_ENGINE_CORE_INCLUDE_AUDIO_DEVICE_NODE_IMPL_H_
