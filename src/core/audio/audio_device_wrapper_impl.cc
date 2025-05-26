
#include "core/audio/audio_device_wrapper_impl.h"

#include <cstring>

#include "webrtc/api/task_queue/default_task_queue_factory.h"

#include "rtc_base/thread.h"
#include "util/log.h"
#ifdef WEBRTC_ANDROID
#include "sdk/android/native_api/audio_device_module/audio_device_android.h"
#endif

namespace core {
constexpr char kLogTag[] = "AudioDeviceWrapperImpl";

using RecordCBType = util::CallbackRegisterTemplate<RecordingEventCallBack>;
using PlayoutCBType = util::CallbackRegisterTemplate<PlayoutEventCallback>;
using RecordSinkType = util::CallbackRegisterTemplate<core::AudioRawSink>;
using PlayoutSourceType = util::CallbackRegisterTemplate<core::AudioRawSource>;

AudioDeviceWrapperImpl::AudioDeviceWrapperImpl() {
  task_queue_factory_ = webrtc::CreateDefaultTaskQueueFactory();
  signal_thread_ = rtc::Thread::Create();
  signal_thread_->SetName("Device_Signal_Thread", nullptr);
  signal_thread_->Start();
  signal_thread_->BlockingCall([this] {
    this->ReCreateAudioDeviceIfNeed(
        webrtc::AudioDeviceModule::kPlatformDefaultAudio);
  });
  audio_mixer_ = AudioMixer::Create([this]() {
    this->signal_thread_->PostTask([this] { this->CheckIfStopPlayout(); });
  });
}
AudioDeviceWrapperImpl::~AudioDeviceWrapperImpl() {
  signal_thread_->BlockingCall([this] {
    if (this->adm_) {
      if (this->adm_->Recording()) {
        this->adm_->StopRecording();
      }
      if (this->adm_->Playing()) {
        this->adm_->StopPlayout();
      }
      this->adm_->Terminate();
      this->adm_ = nullptr;
    }
  });
  signal_thread_->Stop();
}

int32_t AudioDeviceWrapperImpl::EnableBuiltInAec(bool enableaec) {
  return signal_thread_->BlockingCall([this, enableaec] {
    this->enable_buildin_aec_ = enableaec;
    this->adm_->EnableBuiltInAEC(enableaec);
    return 0;
  });
}
int32_t AudioDeviceWrapperImpl::EnableLowLatancy(bool enable) {
  return signal_thread_->BlockingCall([] { return -1; });
}

const core::AudioFormatInfo&
AudioDeviceWrapperImpl::GetDefaultRecordAudioFormatInfo() {
  return core::k48kMonoPcm16;
}

const core::AudioFormatInfo&
AudioDeviceWrapperImpl::GetDefaultPlayoutAudioFormatInfo() {
  return core::k48kStereoPcm16;
}

int32_t AudioDeviceWrapperImpl::AddRecordSink(core::AudioRawSink* sink) {
  {
    std::unique_lock<std::shared_mutex> lock(shared_mutex_);
    RecordSinkType::RegisterCallback(sink);
  }
  signal_thread_->PostTask([this] { this->CheckIfStartRecording(); });
  LOGI_TAG(kLogTag) << "AddRecordSink: " << sink;
  return 0;
}

int32_t AudioDeviceWrapperImpl::RemoveRecordSink(core::AudioRawSink* sink) {
  {
    std::unique_lock<std::shared_mutex> lock(shared_mutex_);
    RecordSinkType::UnRegisterCallback(sink);
  }
  signal_thread_->PostTask([this] { this->CheckIfStopRecording(); });
  LOGI_TAG(kLogTag) << "RemoveRecordSink: " << sink;
  return 0;
}

int32_t AudioDeviceWrapperImpl::AddPlayoutSink(core::AudioRawSink* sink) {
  std::unique_lock<std::shared_mutex> lock(shared_mutex_);
  playout_sink_.RegisterCallback(sink);
  return 0;
}

int32_t AudioDeviceWrapperImpl::RemovePlayoutSink(core::AudioRawSink* sink) {
  std::unique_lock<std::shared_mutex> lock(shared_mutex_);
  playout_sink_.UnRegisterCallback(sink);
  return 0;
}

int32_t AudioDeviceWrapperImpl::AddPlayoutSource(core::AudioRawSource* source) {
  this->audio_mixer_->AddSource(source, [this, source] {
    std::shared_lock<std::shared_mutex> lock(shared_mutex_);
    for (auto& playout_event : this->PlayoutCBType::callback_list_) {
      if (playout_event) {
        playout_event->OnAudioSourceAutoRemoved(source);
      }
    }
  });
  signal_thread_->PostTask([this] { this->CheckIfStartPlayout(); });
  LOGI_TAG(kLogTag) << "AddPlayoutSource: " << source << " source count: "
                    << audio_mixer_->SourceCount();
  return 0;
}

int32_t AudioDeviceWrapperImpl::RemovePlayoutSource(
    core::AudioRawSource* source) {
  this->audio_mixer_->RemoveSource(source);
  signal_thread_->PostTask([this] { this->CheckIfStopPlayout(); });
  LOGI_TAG(kLogTag) << "RemovePlayoutSource: " << source << " source count: "
                    << audio_mixer_->SourceCount();
  return 0;
}
int AudioDeviceWrapperImpl::AddRecordingEventSubscriber(
    RecordingEventCallBack* callback) {
  std::unique_lock<std::shared_mutex> lock(shared_mutex_);
  RecordCBType::RegisterCallback(callback);
  return 0;
}

int AudioDeviceWrapperImpl::RemoveRecordingEventSubscriber(
    RecordingEventCallBack* callback) {
  std::unique_lock<std::shared_mutex> lock(shared_mutex_);
  RecordCBType::UnRegisterCallback(callback);
  return 0;
}

int AudioDeviceWrapperImpl::AddPlayoutEventSubscriber(
    PlayoutEventCallback* callback) {
  std::unique_lock<std::shared_mutex> lock(shared_mutex_);
  PlayoutCBType::RegisterCallback(callback);
  return 0;
}

int AudioDeviceWrapperImpl::RemovePlayoutEventSubscriber(
    PlayoutEventCallback* callback) {
  std::unique_lock<std::shared_mutex> lock(shared_mutex_);
  PlayoutCBType::UnRegisterCallback(callback);
  return 0;
}

util::MillisecondsClass AudioDeviceWrapperImpl::GetPlayoutDelayMs() {
#if defined(WEBRTC_MAC) || defined(WEBRTC_WIN) || defined(WEBRTC_IOS)
  return 20_ms;
#else
  return 150_ms;
#endif
}

int32_t AudioDeviceWrapperImpl::RecordedDataIsAvailable(
    const void* audioSamples, const size_t nSamples,
    const size_t nBytesPerSample, const size_t nChannels,
    const uint32_t samplesPerSec, const uint32_t totalDelayMS,
    const int32_t clockDrift, const uint32_t currentMicLevel,
    const bool keyPressed, uint32_t& newMicLevel) {
  auto write_size = nSamples * nBytesPerSample * nChannels;
  std::shared_lock<std::shared_mutex> lock(shared_mutex_);
  for (auto& sink : RecordSinkType::GetCallbackList()) {
    if (sink) {
      sink->Write(audioSamples, write_size);
    }
  }
  if (first_record_frame_) {
    first_record_frame_ = false;
    for (auto& record_event : RecordCBType::GetCallbackList()) {
      if (record_event) {
        record_event->OnFirstAudioFrame();
      }
    }
  }
  return 0;
}

int32_t AudioDeviceWrapperImpl::NeedMorePlayData(
    const size_t nSamples, const size_t nBytesPerSample, const size_t nChannels,
    const uint32_t samplesPerSec, void* audioSamples, size_t& nSamplesOut,
    int64_t* elapsed_time_ms, int64_t* ntp_time_ms) {
  auto should_read_size = nSamples * nBytesPerSample;
  auto read_size =
      audio_mixer_->Read(audioSamples, should_read_size,
                         AudioFormatInfo(samplesPerSec, nChannels));
  if (read_size < should_read_size) {
    memset(reinterpret_cast<uint8_t*>(audioSamples) + read_size, 0,
           should_read_size - read_size);
  }
  nSamplesOut = read_size * nChannels / nBytesPerSample;
  std::shared_lock<std::shared_mutex> lock(shared_mutex_);
  for (auto& sink : playout_sink_.GetCallbackList()) {
    if (sink) {
      sink->Write(audioSamples, read_size);
    }
  }
  if (first_playout_frame_) {
    first_playout_frame_ = false;
    for (auto& playout_event : PlayoutCBType::GetCallbackList()) {
      if (playout_event) {
        playout_event->OnFirstAudioFrame();
      }
    }
  }
  return 0;
}

#if (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)) || defined(WEBRTC_WIN)
static char s_name[webrtc::kAdmMaxDeviceNameSize];
static char s_guid[webrtc::kAdmMaxGuidSize];
std::vector<std::pair<std::string, std::string>>
AudioDeviceWrapperImpl::GetRecordDeviceList() {
  std::vector<std::pair<std::string, std::string>> res;
  signal_thread_->BlockingCall([&] { res = GetRecordDeviceListInternal(); });
  return res;
}

std::vector<std::pair<std::string, std::string>>
AudioDeviceWrapperImpl::GetPlayoutDeviceList() {
  std::vector<std::pair<std::string, std::string>> res;
  signal_thread_->BlockingCall([&] { res = GetPlayoutDeviceListInternal(); });
  return res;
}

// static char sg_guid_str[256];

int32_t AudioDeviceWrapperImpl::SetRecordingDevice(const char* guid) {
  return signal_thread_->BlockingCall([this, guid] {
    auto device_list = GetRecordDeviceListInternal();
    for (size_t i = 0; i < device_list.size(); ++i) {
      if (device_list[i].second == guid) {
        if (0 != adm_->SetRecordingDevice(i)) {
          return -1;
        } else {
          return 0;
        }
      }
    }
    return -1;
  });
}

int32_t AudioDeviceWrapperImpl::SetPlayoutDevice(const char* guid) {
  return signal_thread_->BlockingCall([this, guid] {
    auto device_list = GetPlayoutDeviceListInternal();
    for (size_t i = 0; i < device_list.size(); ++i) {
      if (device_list[i].second == guid) {
        if (0 != adm_->SetPlayoutDevice(i)) {
          return -1;
        } else {
          return 0;
        }
      }
    }
    return -1;
  });
}
#endif  // (defined(WEBRTC_MAC) && defined(WEBRTC_IOS)) || defined(WEBRTC_WIN)
int AudioDeviceWrapperImpl::Terminate() { return adm_->Terminate(); }
void AudioDeviceWrapperImpl::CheckIfStartRecording() {
  if (adm_ && !adm_->Recording()) {
    bool res = true;
#if defined(WEBRTC_MAC_ONLY) || defined(WEBRTC_WIN)
    adm_->SetRecordingDevice(record_index_);
#endif
    if (0 != adm_->InitRecording() || 0 != adm_->StartRecording()) {
      LOGE_TAG(kLogTag) << "StartRecording failed";
      res = false;
    }
    std::shared_lock<std::shared_mutex> lock(shared_mutex_);
    for (auto& record_event : RecordCBType::GetCallbackList()) {
      if (record_event) {
        record_event->OnRecordingStartResult(res);
      }
    }
  }
}
void AudioDeviceWrapperImpl::CheckIfStopRecording() {
  std::shared_lock<std::shared_mutex> lock(shared_mutex_);
  if (RecordCBType::GetCallbackList().empty()) {
    if (adm_ && adm_->Recording()) {
      if (adm_->StopRecording() == 0) {
        for (auto& record_event : RecordCBType::GetCallbackList()) {
          if (record_event) {
            record_event->OnRecordingStop();
          }
        }
      }
    }
  }
}
void AudioDeviceWrapperImpl::CheckIfStartPlayout() {
  if (adm_ && !adm_->Playing()) {
    bool res = true;
#if defined(WEBRTC_MAC_ONLY) || defined(WEBRTC_WIN)
    adm_->SetPlayoutDevice(playout_index_);
#endif
    LOGI_TAG(kLogTag) << "StartPlayout";
    if (0 != adm_->InitPlayout() || 0 != adm_->StartPlayout()) {
      LOGE_TAG(kLogTag) << "StartPlayout failed";
      res = false;
    } else {
      LOGI_TAG(kLogTag) << "StartPlayout success";
    }
    std::shared_lock<std::shared_mutex> lock(shared_mutex_);
    for (auto& playout_event : PlayoutCBType::GetCallbackList()) {
      if (playout_event) {
        playout_event->OnPlayoutStartResult(res);
      }
    }
  }
}
void AudioDeviceWrapperImpl::CheckIfStopPlayout() {
  if (adm_ && audio_mixer_->SourceCount() == 0) {
    // if (adm_) {
    if (adm_->Playing()) {
      LOGI_TAG(kLogTag) << "StopPlayout";
      if (adm_->StopPlayout() == 0) {
        std::shared_lock<std::shared_mutex> lock(shared_mutex_);
        for (auto& playout_event : PlayoutCBType::GetCallbackList()) {
          if (playout_event) {
            playout_event->OnPlayoutStop();
          }
        }
        LOGI_TAG(kLogTag) << "StopPlayout success";
      } else {
        LOGE_TAG(kLogTag) << "StopPlayout failed";
      }
    }
  }
}

void AudioDeviceWrapperImpl::ReCreateAudioDeviceIfNeed(
    webrtc::AudioDeviceModule::AudioLayer audio_layer) {
  LOGI_TAG(kLogTag) << "ReCreateAudioDeviceIfNeed: " << audio_layer_ << " "
                    << audio_layer;
  if (nullptr == adm_ || audio_layer != audio_layer_) {
    this->audio_layer_ = audio_layer;
    if (nullptr != this->adm_) {
      this->adm_ = nullptr;
    }
#ifdef MEDIA_ANDROID
    this->adm_ = webrtc::CreateAndroidAudioDeviceModule(audio_layer_);
#else
    this->adm_ = webrtc::AudioDeviceModule::Create(audio_layer_,
                                                   task_queue_factory_.get());
#endif
    this->adm_->Init();
    this->adm_->RegisterAudioCallback(this);
  }
}

#if defined(WEBRTC_MAC) || defined(WEBRTC_WIN)
std::vector<std::pair<std::string, std::string>>
AudioDeviceWrapperImpl::GetRecordDeviceListInternal() {
  std::vector<std::pair<std::string, std::string>> res;
  int num_devices = this->adm_->RecordingDevices();
  for (int i = 0; i < num_devices; ++i) {
    this->adm_->RecordingDeviceName(i, s_name, s_guid);
    res.push_back({s_name, s_guid});
  }
  return res;
}

std::vector<std::pair<std::string, std::string>>
AudioDeviceWrapperImpl::GetPlayoutDeviceListInternal() {
  std::vector<std::pair<std::string, std::string>> res;
  int num_devices = this->adm_->PlayoutDevices();
  for (int i = 0; i < num_devices; ++i) {
    this->adm_->PlayoutDeviceName(i, s_name, s_guid);
    res.push_back({s_name, s_guid});
  }
  return res;
}
#endif

AudioDeviceWrapper& AudioDeviceWrapper::GetInstance() {
  static AudioDeviceWrapperImpl s_instance;
  return s_instance;
}
}  // namespace core
