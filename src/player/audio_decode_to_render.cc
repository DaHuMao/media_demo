#include "player/audio_decode_to_render.h"
#include "core/adapter/audio_frame_ffmpeg.h"
namespace media_demo {
int AudioDecodeToRender::Init(const AudioDecodeToRender::Config& config) {
  if (DecodeToRender::Init(config) != 0) {
    return -1;
  }
  config_ = config;
  return 0;
}


int32_t AudioDecodeToRender::OnFrame(const FrameInfo* frame) {
  if (config_.audio_render->SendFrame(
        core::AudioFrameFfmpeg::GetAudioFrameLiteView(frame), false) == 0) {
    config_.timer->SetAudioCurrentTime(frame->GetPtsMs().Value() -
        config_.audio_render->GetDelayMs(), frame->GetSerial());
    return 0;
  }
  return std::max(frame->GetDurationMs().Value(), static_cast<int64_t>(20));
}

int AudioDecodeToRender::UnInit() {
  if (!is_init_) {
    return 0;
  }
  DecodeToRender::UnInit();
  is_init_ = false;
  return 0;
}

}  // namespace media_demo
