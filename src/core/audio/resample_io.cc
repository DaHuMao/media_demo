#include "core/audio/resample_io.h"
namespace core {
AudioResampleAudioSink::AudioResampleAudioSink(
    util::PointDelegate<core::AudioRawSink> sink)
    : sink_(std::move(sink)) {}

int AudioResampleAudioSink::Write(const void *data_ptr, size_t data_size,
                                  const core::AudioFormatInfo &info) {
  auto &info_wanted = sink_->GetNeededAudioFormatInfo();
  if (info != info_wanted) {
    if (audio_converter_ == nullptr) {
      audio_converter_ = std::make_unique<AudioConvertWrapper>(info_wanted);
    }
    audio_converter_->Convert(info, data_ptr, data_size);
    auto &dst = audio_converter_->GetAudioFrame();
    return sink_->Write(dst.Data(), dst.ByteSize());
  } else {
    return sink_->Write(data_ptr, data_size);
  }
}

int AudioResampleAudioSink::Write(
    const core::AudioFrameLiteView &audio_frame) {
  if (audio_converter_ == nullptr) {
    audio_converter_ = std::make_unique<AudioConvertWrapper>(
        sink_->GetNeededAudioFormatInfo());
  }
  audio_converter_->Convert(audio_frame);
  auto &dst = audio_converter_->GetAudioFrame();
  return sink_->Write(dst.Data(), dst.ByteSize());
}

AudioResampleRawSink::AudioResampleRawSink(
    util::PointDelegate<core::AudioRawSink> sink,
    const core::AudioFormatInfo &source_info)
    : resample_audio_sink_(std::move(sink)), source_info_(source_info) {}

size_t AudioResampleRawSink::Write(const void *data_ptr, size_t data_size) {
  return resample_audio_sink_.Write(data_ptr, data_size, source_info_);
}

size_t AudioResampleAudioSource::Read(void *data, size_t read_size,
                                      const AudioFormatInfo &info) {
  if (!source_) {
    return 0;
  }
  size_t ret = 0;
  if (audio_frame_buffer_.AudioFormat() != info) {
    AudioFrameLiteDelegate audio_frame(reinterpret_cast<uint8_t *>(data), 0,
                                       read_size, info);
    auto need_size = audio_frame_buffer_.AudioFormat().AudioConvertByteSize(
        info, read_size);
    audio_frame_buffer_.ExpandCapacityIfNeed(need_size);
    audio_frame_buffer_.ResetReadableSizeInByte(
        source_->Read(audio_frame_buffer_.MutableData(), need_size));
    audio_converter_.Convert(audio_frame_buffer_, audio_frame);
    ret = audio_frame.ByteSize();
  } else {
    ret = source_->Read(data, read_size);
  }
  return ret;
}

size_t AudioResampleAudioSource::CurrentSize(const AudioFormatInfo &tar_info) {
  if (source_) {
    return source_->CurrentSize() * tar_info.ByteSizePerFrame() /
           source_->GetAudioFormatInfo().ByteSizePerFrame();
  }
  return 0;
}

core::SourceStatus AudioResampleAudioSource::GetSourceStatus() const {
  return source_ ? source_->GetSourceStatus() : core::SourceStatus::kError;
}

int AudioResampleAudioSource::DiscardDataSizeInByte(size_t offset) {
  return source_ ? source_->DiscardDataSizeInByte(offset) : -1;
}

int AudioResampleAudioSource::FillZeroFront(util::MillisecondsClass time_ms) {
  return source_ ? source_->FillZeroFront(time_ms) : -1;
}

size_t AudioResampleAudioRawSource::Read(void *data_ptr, size_t data_size) {
  return resample_audio_source_.Read(data_ptr, data_size, tar_info_);
}

int AudioResampleAudioRawSource::DiscardDataSizeInByte(size_t offset) {
  return resample_audio_source_.DiscardDataSizeInByte(offset);
}

size_t AudioResampleAudioRawSource::CurrentSize() {
  return resample_audio_source_.CurrentSize(tar_info_);
}

core::SourceStatus AudioResampleAudioRawSource::GetSourceStatus() const {
  return resample_audio_source_.GetSourceStatus();
}

util::MillisecondsClass AudioResampleAudioRawSource::CurrentSizeMs() {
  return tar_info_.AudioByteSizeToMs(
      resample_audio_source_.CurrentSize(tar_info_));
}

int AudioResampleAudioRawSource::FillZeroFront(
    util::MillisecondsClass time_ms) {
  return resample_audio_source_.FillZeroFront(time_ms);
}

}  // namespace core
