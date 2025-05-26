#ifndef PLAYER_AUDIO_RENDER_IMPL_H_
#define PLAYER_AUDIO_RENDER_IMPL_H_
#include <memory>
#include <mutex>

#include "core/audio/audio_io_define.h"
#include "core/audio/audio_ring_buffer_io.h"
#include "player/audio_render.h"
#include "util/time_to_class.h"
namespace media_demo {
class AudioRenderImpl : public core::AudioRawSource, public AudioRender {
 public:
  AudioRenderImpl(const core::AudioFormatInfo& possible_audio_format);
  ~AudioRenderImpl();
  size_t Read(void* data_ptr, size_t data_size) override;
  int DiscardDataSizeInByte(size_t offset) override;
  size_t CurrentSize() override;
  core::SourceStatus GetSourceStatus() const override;
  const core::AudioFormatInfo& GetAudioFormatInfo() const override;
  util::MillisecondsClass CurrentSizeMs() override;
  int FillZeroFront(util::MillisecondsClass time_ms) override;
  int Init() override;
  int Play() override;
  int Pause() override;
  int Flush() override;
  int UnInit() override;
  int SendFrame(const core::AudioFrameMaybePlanarView& frame,
                bool is_blocking) override;
  int GetDelayMs() override;

 private:
  std::mutex mutex_;
  std::atomic_bool is_init_ = false;
  std::atomic_bool is_playing_ = false;
  std::atomic_bool player_start_watermark_had_reached_ = false;
  // 起播水位
  util::MillisecondsClass player_start_watermark_;
  std::unique_ptr<core::AudioRingBufferIo> audio_ring_buffer_io_;
};
}  // namespace media_demo
#endif  // PLAYER_AUDIO_RENDER_IMPL_H_
