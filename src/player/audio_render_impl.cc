
#include "player/audio_render_impl.h"

#include "core/audio/audio_device_wrapper_impl.h"
#include "util/log.h"
namespace media_demo {
const util::MillisecondsClass kPlayerStartWatermark = 200_ms;
constexpr char kTag[] = "AudioRenderImpl";
AudioRenderImpl::AudioRenderImpl(
    const core::AudioFormatInfo& possible_audio_format)
    : player_start_watermark_(kPlayerStartWatermark) {
  audio_ring_buffer_io_ = core::AudioRingBufferIo::Create(
      possible_audio_format, possible_audio_format.AudioMsToByteSize(500_ms));
}

AudioRenderImpl::~AudioRenderImpl() { UnInit(); }
util::MillisecondsClass now_time = util::TimeNow();
size_t AudioRenderImpl::Read(void* data_ptr, size_t data_size) {
  now_time = util::TimeNow();
  if (is_playing_ && player_start_watermark_had_reached_) {
    auto read_size = audio_ring_buffer_io_->Read(data_ptr, data_size);
    return read_size;
  }
  return 0;
}

int AudioRenderImpl::DiscardDataSizeInByte(size_t offset) {
  return audio_ring_buffer_io_->DiscardDataSizeInByte(offset);
}

size_t AudioRenderImpl::CurrentSize() {
  return audio_ring_buffer_io_->CurrentSize();
}

core::SourceStatus AudioRenderImpl::GetSourceStatus() const {
  return audio_ring_buffer_io_->GetSourceStatus();
}

const core::AudioFormatInfo& AudioRenderImpl::GetAudioFormatInfo() const {
  return audio_ring_buffer_io_->GetAudioFormatInfo();
}

util::MillisecondsClass AudioRenderImpl::CurrentSizeMs() {
  return audio_ring_buffer_io_->CurrentSizeMs();
}

int AudioRenderImpl::FillZeroFront(util::MillisecondsClass time_ms) {
  return -1;
}

int AudioRenderImpl::Init() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!is_init_) {
    auto& audio_device = core::AudioDeviceWrapper::GetInstance();
    audio_device.AddPlayoutSource(this);
    auto buffer_capacity =
        std::max(player_start_watermark_ + audio_device.GetPlayoutDelayMs(),
                 audio_device.GetPlayoutDelayMs() * 2);
    audio_ring_buffer_io_->ResetUseableCapacity(
        audio_ring_buffer_io_->GetAudioFormatInfo().AudioMsToByteSize(
            buffer_capacity));
    LOGI_TAG(kTag) << "Init AudioRenderImpl"
                   << "audio format: "
                   << audio_ring_buffer_io_->GetAudioFormatInfo()
                   << " buffer capacity: " << buffer_capacity << " ms";
    is_init_ = true;
  }
  return 0;
}

int AudioRenderImpl::Play() {
  is_playing_ = true;
  return 0;
}

int AudioRenderImpl::Pause() {
  is_playing_ = false;
  return 0;
}

int AudioRenderImpl::Flush() {
  audio_ring_buffer_io_->Reset();
  player_start_watermark_had_reached_ = false;
  return 0;
}

int AudioRenderImpl::UnInit() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_init_) {
    is_init_ = false;
    is_playing_ = false;
    core::AudioDeviceWrapper::GetInstance().RemovePlayoutSource(this);
  }
  return 0;
}

int AudioRenderImpl::SendFrame(const core::AudioFrameMaybePlanarView& frame,
                               bool is_blocking) {
  RTC_DCHECK(frame.AudioFormat() ==
             audio_ring_buffer_io_->GetAudioFormatInfo());
  if (audio_ring_buffer_io_->FreeSpaceBeforeOverwritingMs() >=
      frame.SizeInMs() + 1_ms) {
    audio_ring_buffer_io_->WritePlanar(frame.PlanarData(),
                                       frame.OneDimByteSize());
    auto vec = frame.PlanarData();
    if (audio_ring_buffer_io_->CurrentSizeMs() >= player_start_watermark_) {
      player_start_watermark_had_reached_ = true;
    }
    return 0;
  }
  return -1;
}

int AudioRenderImpl::GetDelayMs() {
  return (audio_ring_buffer_io_->CurrentSizeMs() +
          core::AudioDeviceWrapper::GetInstance().GetPlayoutDelayMs())
      .Value();
}

std::unique_ptr<AudioRender> AudioRender::Create(
    const core::AudioFormatInfo& audio_format) {
  return std::make_unique<AudioRenderImpl>(audio_format);
}

}  // namespace media_demo
