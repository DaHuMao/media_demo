#ifndef PLAYER_AUDIO_DECODE_TO_RENDER_H_
#define PLAYER_AUDIO_DECODE_TO_RENDER_H_
#include "player/audio_render.h"
#include "player/decode_to_render.h"
#include "player/player_timer.h"
namespace media_demo {
class AudioDecodeToRender final : public DecodeToRender {
 public:
  struct Config : public DecodeToRender::Config {
    PlayerTimer* timer = nullptr;
    AudioRender* audio_render = nullptr;
  };
  AudioDecodeToRender() = default;
  ~AudioDecodeToRender() override = default;
  int Init(const Config& config);
  int UnInit();

 private:
  int32_t OnFrame(const FrameInfo* frame) override;
  bool is_init_ = false;
  Config config_;
};
}  // namespace media_demo
#endif  // PLAYER_AUDIO_DECODE_TO_RENDER_H_
