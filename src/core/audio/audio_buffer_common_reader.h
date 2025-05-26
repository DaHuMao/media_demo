#ifndef WEBRTC_AUDIO_AUDIO_BUFFER_COMMON_READER_H_
#define WEBRTC_AUDIO_AUDIO_BUFFER_COMMON_READER_H_
#include <vector>

#include "core/audio/audio_io_define.h"
#include "util/ring_buffer_wrapper.h"
namespace core {
class AudioBufferCommonReader {
 public:
  AudioBufferCommonReader(size_t buffer_count, size_t buffer_capacity,
                          bool auto_expand_capacity = false);
  virtual ~AudioBufferCommonReader();
  AudioBufferCommonReader(const AudioBufferCommonReader&) = delete;
  AudioBufferCommonReader(AudioBufferCommonReader&&) = delete;

  size_t BaseRead(void* data_ptr, size_t data_size);
  int BaseDiscardDataSizeInByte(size_t data_size);
  int BaseReset();
  int BaseFillZeroFront(size_t data_size);
  size_t BaseCurrentSize() const;
  SourceStatus BaseGetSourceStatus() const;

 protected:
  size_t GetSingleBufferMinSize() const;
  size_t CurrentSizeInternal() const;
  virtual void CheckStatus() {}

 protected:
  std::atomic<SourceStatus> status_;
  int32_t should_seek_size_ = 0;
  std::vector<std::unique_ptr<core::RingBufferWrapper>> circular_buffer_vec_;

 private:
  void ReadForInterweave(int should_seek_size, int should_read_size,
                         void* read_ptr);
  void ReadForPlanner(int should_seek_size, int should_read_size,
                      void* read_ptr);
};
}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_BUFFER_COMMON_READER_H_
