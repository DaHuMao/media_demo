#ifndef WEBRTC_AUDIO_AUDIO_RING_BUFFER_IO_IMPL_H_
#define WEBRTC_AUDIO_AUDIO_RING_BUFFER_IO_IMPL_H_
#include <condition_variable>

#include "core/audio/audio_buffer_common_reader.h"
#include "core/audio/audio_ring_buffer_io.h"
namespace core {
class AudioRingBufferIoImpl final : public AudioRingBufferIo,
                                    private AudioBufferCommonReader {
 public:
  AudioRingBufferIoImpl(const AudioRingBufferIoImpl &) = delete;
  AudioRingBufferIoImpl(AudioRingBufferIoImpl &&) = delete;

  AudioRingBufferIoImpl(const AudioFormatInfo &info, size_t buffer_capacity,
                        bool auto_expand_capacity = false);

  ~AudioRingBufferIoImpl() override = default;
  size_t Read(void *data_ptr, size_t data_size) override;
  int DiscardDataSizeInByte(size_t pos) override;
  size_t CurrentSize() override;
  util::MillisecondsClass CurrentSizeMs() override {
    return format_info_.AudioByteSizeToMs(CurrentSize());
  }
  SourceStatus GetSourceStatus() const override;
  const AudioFormatInfo &GetAudioFormatInfo() const override;
  size_t Write(const void *data_ptr, size_t data_size) override;
  void WriteCompletion() override;
  const AudioFormatInfo &GetNeededAudioFormatInfo() const override;
  int FillZeroFront(util::MillisecondsClass time_ms) override;

  void ResetUseableCapacity(size_t size) override;
  size_t BlockingWrite(const void *data, int size) override;
  size_t BlockingRead(void *data, int size) override;
  size_t FreeSpaceBeforeOverwriting() override;
  util::MillisecondsClass FreeSpaceBeforeOverwritingMs() override;
  void CancelRead() override;
  void CancelWrite() override;
  void CancelAll() override;
  int Reset() override;
  size_t WritePlanar(const std::vector<const uint8_t *> &data,
                     size_t size_byte_every_dim) override;

  size_t FreeSpaceBeforeOverwritingNoLock();

 private:
  size_t ReadInternal(void *data_ptr, size_t data_size, bool is_blocking);
  size_t WriteInternal(const void *data_ptr, size_t data_size,
                       bool is_blocking);
  size_t WritePlanarInternal(const std::vector<const uint8_t *> &data,
                             size_t size_byte_evert_dim, bool is_blocking);
  void WaitingWrite(size_t should_write_size, std::unique_lock<std::mutex> &lk);
  void WaitingRead(size_t should_read_size, std::unique_lock<std::mutex> &lk);
  void CheckIfNotifyRead();
  void CheckIfNotifyWrite();
  void NotifyReader();
  void NotifyWriter();

 private:
  bool cancel_read_ = false;
  bool cancel_write_ = false;
  bool is_auto_expand_capacity_ = false;
  const AudioFormatInfo format_info_;
  std::mutex mutex_;
  std::condition_variable cv_reader_;
  std::condition_variable cv_writer_;
  size_t waiting_read_size_ = 0;
  size_t waiting_write_size_ = 0;
  size_t buffer_useable_capacity_ = 0;
};
}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_RING_BUFFER_IO_IMPL_H_
