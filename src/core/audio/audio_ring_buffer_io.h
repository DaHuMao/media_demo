#ifndef WEBRTC_AUDIO_AUDIO_RING_BUFFER_IO_H_
#define WEBRTC_AUDIO_AUDIO_RING_BUFFER_IO_H_
#include "core/audio/audio_io_define.h"
namespace core {
class AudioRingBufferIo : public core::AudioRawSource,
                          public core::AudioRawSink {
 public:
  virtual ~AudioRingBufferIo() = default;
  /*
   * 如果当前buffer的容量小于size:
   *    1. buffer的内存会被重置,所以可读数据会被拷贝到新的buffer中
   * 如果当前buffer的容量大于size:
   *    1. buffer的内存不会被重置, 只是会设置一个buffer的使用容量.
   *    2. 此时buffer的可读数据并不会变化，所以可能出现buffer的可读数据大于buffer的使用容量
   *    3. 可写数据不会超过buffer的使用容量
   */
  virtual void ResetUseableCapacity(size_t size) = 0;
  virtual size_t BlockingWrite(const void* data, int size) = 0;
  virtual size_t BlockingRead(void* data, int size) = 0;
  virtual size_t FreeSpaceBeforeOverwriting() = 0;
  virtual util::MillisecondsClass FreeSpaceBeforeOverwritingMs() = 0;
  virtual void CancelRead() = 0;
  virtual void CancelWrite() = 0;
  virtual void CancelAll() = 0;
  virtual int Reset() = 0;
  /*
   * 当audio_format是planner时，data.size() == audio_format.GetChannelsCount()
   * @data: 一帧音频数据
   * @one_data_size: data中单个通道的数据大小
   */
  virtual size_t WritePlanar(const std::vector<const uint8_t*>& data,
                             size_t size_byte_every_dim) = 0;
  static std::unique_ptr<AudioRingBufferIo> Create(
      const core::AudioFormatInfo& audio_format, uint32_t size_in_byte);

  // 这种创建模式下，buffer的容量会自动扩展
  // 因此ResetUseableCapacity的调用会被忽略
  static std::unique_ptr<AudioRingBufferIo> CreateAutoExpandBuffer(
      const core::AudioFormatInfo& audio_format, uint32_t size_in_byte);
};
}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_RING_BUFFER_IO_H_
