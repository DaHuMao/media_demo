#ifndef PLAYER_AUDIO_RENDER_H_
#define PLAYER_AUDIO_RENDER_H_
#include "core/audio/audio_frame.h"
namespace media_demo {
class AudioRender {
 public:
  virtual ~AudioRender() = default;
  virtual int Init() = 0;
  virtual int Play() = 0;
  virtual int Pause() = 0;
  virtual int Flush() = 0;
  virtual int UnInit() = 0;
  // 0: success, -1: failed
  virtual int SendFrame(const core::AudioFrameMaybePlanarView& frame,
                        bool is_blocking) = 0;
  virtual int GetDelayMs() = 0;
  static std::unique_ptr<AudioRender> Create(
      const core::AudioFormatInfo& audio_format);
};
}  // namespace media_demo
#endif  // PLAYER_AUDIO_RENDER_H_
