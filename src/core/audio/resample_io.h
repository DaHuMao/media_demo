#include <vector>

#include "core/audio/audio_convert_wrapper.h"
#include "core/audio/audio_converter.h"
#include "core/audio/audio_frame.h"
#include "core/audio/audio_io_define.h"
#include "util/point_delegate.h"
namespace core {
class AudioResampleAudioSink : public core::AudioSink {
 public:
  AudioResampleAudioSink(util::PointDelegate<core::AudioRawSink> sink);
  AudioResampleAudioSink(AudioResampleAudioSink &&) = default;
  AudioResampleAudioSink &operator=(AudioResampleAudioSink &&) = default;
  int Write(const void *data_ptr, size_t data_size,
            const core::AudioFormatInfo &info) override;
  void WriteCompletion() override { sink_->WriteCompletion(); }
  int Write(const core::AudioFrameLiteView &audio_frame);

 private:
  AudioResampleAudioSink(const AudioResampleAudioSink &) = delete;
  AudioResampleAudioSink &operator=(const AudioResampleAudioSink &) = delete;
  util::PointDelegate<core::AudioRawSink> sink_;
  std::unique_ptr<AudioConvertWrapper> audio_converter_;
};

class AudioResampleRawSink : public core::AudioRawSink {
 public:
  AudioResampleRawSink(util::PointDelegate<core::AudioRawSink> sink,
                       const core::AudioFormatInfo &source_info);
  AudioResampleRawSink(AudioResampleRawSink &&) = default;
  AudioResampleRawSink &operator=(AudioResampleRawSink &&) = default;
  size_t Write(const void *data_ptr, size_t data_size) override;
  void WriteCompletion() override { resample_audio_sink_.WriteCompletion(); }
  const core::AudioFormatInfo &GetNeededAudioFormatInfo() const override {
    return source_info_;
  }

 private:
  AudioResampleRawSink(const AudioResampleRawSink &) = delete;
  AudioResampleRawSink &operator=(const AudioResampleRawSink &) = delete;
  AudioResampleAudioSink resample_audio_sink_;
  core::AudioFormatInfo source_info_;
};

class AudioResampleAudioSource {
 public:
  AudioResampleAudioSource(util::PointDelegate<core::AudioRawSource> source)
      : source_(std::move(source)),
        audio_converter_(true),
        audio_frame_buffer_(source_ ? source_->GetAudioFormatInfo()
                                    : core::kNullAudioFormatInfo,
                            10_ms) {}
  size_t Read(void *data, size_t read_size, const AudioFormatInfo &info);
  size_t CurrentSize(const AudioFormatInfo &tar_info);
  core::SourceStatus GetSourceStatus() const;
  int DiscardDataSizeInByte(size_t offset);
  int FillZeroFront(util::MillisecondsClass time_ms);

 private:
  util::PointDelegate<core::AudioRawSource> source_;
  core::AudioConverterFFmpeg audio_converter_;
  core::AudioFrameLite audio_frame_buffer_;
};

class AudioResampleAudioRawSource : public core::AudioRawSource {
 public:
  AudioResampleAudioRawSource(util::PointDelegate<core::AudioRawSource> source,
                              const core::AudioFormatInfo &tar_info)
      : resample_audio_source_(std::move(source)), tar_info_(tar_info) {}
  size_t Read(void *data_ptr, size_t data_size) override;
  int DiscardDataSizeInByte(size_t offset) override;
  size_t CurrentSize() override;
  core::SourceStatus GetSourceStatus() const override;
  util::MillisecondsClass CurrentSizeMs() override;
  int FillZeroFront(util::MillisecondsClass time_ms) override;
  const core::AudioFormatInfo &GetAudioFormatInfo() const override {
    return tar_info_;
  }

 private:
  AudioResampleAudioRawSource(const AudioResampleAudioRawSource &) = delete;
  AudioResampleAudioRawSource &operator=(const AudioResampleAudioRawSource &) =
      delete;
  AudioResampleAudioSource resample_audio_source_;
  core::AudioFormatInfo tar_info_;
};
}  // namespace core
