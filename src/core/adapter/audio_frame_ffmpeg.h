#ifndef CORE_ADAPTER_AUDIO_FRAME_FFMPEG_H_
#define CORE_ADAPTER_AUDIO_FRAME_FFMPEG_H_
#include "core/audio/audio_frame.h"
#include "core/ffmpeg/av_frame_wrapper.h"
namespace core {
class AudioFrameFfmpeg {
 public:
  static AudioFrameMaybePlanarView GetAudioFrameLiteView(
      const core::AvFrameWrapper* frame);
};
}  // namespace core
#endif  // CORE_ADAPTER_AUDIO_FRAME_FFMPEG_H_
