#include "core/audio/audio_convert_wrapper.h"
namespace core {
AudioConvertWrapper::AudioConvertWrapper(const core::AudioFormatInfo &dst)
    : audio_frame_(dst) {}
int AudioConvertWrapper::Convert(const core::AudioFormatInfo &src,
                                 const void *data, size_t size) {
  auto need_capacity = (size + src.ByteSizePerFrame() - 1) *
                       audio_frame_.AudioFormat().ByteSizePerFrame() /
                       src.ByteSizePerFrame();
  audio_frame_.ExpandCapacityIfNeed(need_capacity);
  return audio_converter_.Convert(core::AudioFrameLiteView(data, size, &src),
                                  audio_frame_);
}

int AudioConvertWrapper::Convert(const core::AudioFrameLiteView &src) {
  auto need_capacity =
      (src.ByteSize() + src.AudioFormat().ByteSizePerFrame() - 1) *
      audio_frame_.AudioFormat().ByteSizePerFrame() /
      src.AudioFormat().ByteSizePerFrame();
  audio_frame_.ExpandCapacityIfNeed(need_capacity);
  return audio_converter_.Convert(src, audio_frame_);
}

}  // namespace core
