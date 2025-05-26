#include <cstdint>

#include "core/audio/audio_format_define.h"
#include "core/audio/audio_io_define.h"
#include "util/macro_defines.h"
namespace core {
class RecordingEventCallBack {
 public:
  virtual ~RecordingEventCallBack() = default;
  virtual void OnRecordingStartResult(bool is_success) = 0;
  virtual void OnRecordingStop() = 0;
  virtual void OnFirstAudioFrame() = 0;
};

class PlayoutEventCallback {
 public:
  virtual ~PlayoutEventCallback() = default;
  virtual void OnPlayoutStartResult(bool is_success) = 0;
  virtual void OnPlayoutStop() = 0;
  virtual void OnFirstAudioFrame() = 0;
  virtual void OnAudioSourceAutoRemoved(core::AudioRawSource* source) = 0;
};

class AudioDeviceWrapper {
 public:
  static AudioDeviceWrapper& GetInstance();
  virtual ~AudioDeviceWrapper() = default;
  virtual int32_t EnableBuiltInAec(bool enableaec) = 0;
  virtual int32_t EnableLowLatancy(bool enable) = 0;
  virtual const core::AudioFormatInfo& GetDefaultRecordAudioFormatInfo() = 0;
  virtual const core::AudioFormatInfo& GetDefaultPlayoutAudioFormatInfo() = 0;
  virtual int32_t AddRecordSink(core::AudioRawSink* sink) = 0;
  virtual int32_t RemoveRecordSink(core::AudioRawSink* sink) = 0;
  virtual int32_t AddPlayoutSink(core::AudioRawSink* sink) = 0;
  virtual int32_t RemovePlayoutSink(core::AudioRawSink* sink) = 0;
  virtual int32_t AddPlayoutSource(core::AudioRawSource* source) = 0;
  virtual int32_t RemovePlayoutSource(core::AudioRawSource* source) = 0;
  virtual int AddRecordingEventSubscriber(RecordingEventCallBack* callback) = 0;
  virtual int RemoveRecordingEventSubscriber(
      RecordingEventCallBack* callback) = 0;
  virtual int AddPlayoutEventSubscriber(PlayoutEventCallback* callback) = 0;
  virtual int RemovePlayoutEventSubscriber(PlayoutEventCallback* callback) = 0;
  virtual util::MillisecondsClass GetPlayoutDelayMs() = 0;

#if defined(WEBRTC_MAC_ONLY) || defined(WEBRTC_WIN)
  //<name uuid>
  virtual std::vector<std::pair<std::string, std::string>>
  GetRecordDeviceList() = 0;
  virtual std::vector<std::pair<std::string, std::string>>
  GetPlayoutDeviceList() = 0;
  virtual int32_t SetRecordingDevice(const char* guid) = 0;
  virtual int32_t SetPlayoutDevice(const char* guid) = 0;
#endif
};
}  // namespace core
