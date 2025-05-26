/**
 * audio_io_define.h, Sep 2023.
 * created by ZhangTongXiao
 */
#ifndef WEBRTC_AUDIO_AUDIO_IO_DEFINE_H_
#define WEBRTC_AUDIO_AUDIO_IO_DEFINE_H_

#include "core/audio/audio_format_define.h"
#include "util/time_to_class.h"

namespace core {

enum class SourceStatus { kStreaming = 0, kStatic, kError };

class RawSource {
 public:
  virtual ~RawSource() = default;
  /*
   * @return value: min(|data_size|, this->CurrentSize())
   *      In principle, <CurrentSize()> should be called first before this
   * function to get proper |data_size|
   */
  virtual size_t Read(void *data_ptr, size_t data_size) = 0;
  /*
   * @return value:
   *     success 0
   *     failed  < 0
   */
  virtual int DiscardDataSizeInByte(size_t offset) = 0;
  virtual size_t CurrentSize() = 0;
  virtual SourceStatus GetSourceStatus() const = 0;
};

class AudioRawSource : public RawSource {
 public:
  ~AudioRawSource() override = default;
  virtual const AudioFormatInfo &GetAudioFormatInfo() const = 0;
  virtual util::MillisecondsClass CurrentSizeMs() = 0;
  virtual int FillZeroFront(util::MillisecondsClass time_ms) = 0;
};

class RawSink {
 public:
  virtual ~RawSink() = default;
  /*
   * @return value: The size of actual writes
   * */
  virtual size_t Write(const void *data_ptr, size_t data_size) = 0;
  /*
   * When this function is called, it means that no more data will be written to
   * the Sink
   */
  virtual void WriteCompletion() = 0;
};

class AudioRawSink : public RawSink {
 public:
  ~AudioRawSink() override = default;
  virtual const AudioFormatInfo &GetNeededAudioFormatInfo() const = 0;
};

class AudioFileSource : public AudioRawSource {
 public:
  ~AudioFileSource() override = default;
  virtual void SetCyclicMode(bool enable) = 0;
  virtual int Seek(int seek_pos) = 0;
};

class AudioSink {
 public:
  virtual ~AudioSink() = default;
  virtual int Write(const void *data, size_t size,
                    const AudioFormatInfo &audio_format) = 0;
  virtual void WriteCompletion() = 0;
};

}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_IO_DEFINE_H_
