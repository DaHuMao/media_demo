#include "core/audio/file/wav_file_header.h"

#include "webrtc/common_audio/wav_header.h"

#include "util/array_find.h"
namespace core {
static constexpr std::pair<AudioSampleFormat, webrtc::WavFormat>
    kCodecToWavFormat[] = {{AudioSampleFormat::kAudioSampleFormatPcmInt16,
                            webrtc::WavFormat::kWavFormatPcm},
                           {AudioSampleFormat::kAudioSampleFormatPcmFloat,
                            webrtc::WavFormat::kWavFormatIeeeFloat}};
class ReadableWavImpl final : public webrtc::WavHeaderReader {
 public:
  ReadableWavImpl(AudioFileSource* source);

  ~ReadableWavImpl() override = default;
  size_t Read(void* data_ptr, size_t data_size) override;
  bool SeekForward(uint32_t num_bytes) override {
    return reader_ ? reader_->Seek(num_bytes) == 0 : false;
  }

  int64_t GetPosition() override {
    return reader_ ? all_size_ - reader_->CurrentSize() : 0;
  }

 private:
  AudioFileSource* reader_ = nullptr;
  size_t all_size_ = 0;
};
ReadableWavImpl::ReadableWavImpl(AudioFileSource* reader) : reader_(reader) {
  if (reader_) {
    all_size_ = reader_->CurrentSize();
  }
}
size_t ReadableWavImpl::Read(void* data_ptr, size_t data_size) {
  return reader_ ? reader_->Read(data_ptr, data_size) : 0;
}
bool AudioWavFileHeader::ReadHeader(
    AudioFileSource* read_func, AudioFileHeaderInfo* audio_file_header_info) {
  RTC_DCHECK(audio_file_header_info && read_func);
  ReadableWavImpl readable_wav(read_func);
  webrtc::WavFormat wav_fmt;
  size_t bytes_per_sample;
  size_t num_samples;
  int sample_rate_hz;
  size_t num_channels;
  int64_t data_start_pos;
  if (webrtc::ReadWavHeader(&readable_wav, &num_channels, &sample_rate_hz,
                            &wav_fmt, &bytes_per_sample, &num_samples,
                            &data_start_pos)) {
    audio_file_header_info->audio_format_info = AudioFormatInfo(
        sample_rate_hz, static_cast<int>(num_channels),
        util::ArrayFindKey(kCodecToWavFormat, wav_fmt,
                           AudioSampleFormat::kAudioSampleFormatNone));
    // num_samples = bytes_all / bytes_per_sample
    // num_frames = bytes_all / bytes_per_sample / num_channels
    RTC_DCHECK_EQ(num_samples % num_channels, 0);
    audio_file_header_info->audio_frame_size = num_samples / num_channels;
    RTC_DCHECK_EQ(bytes_per_sample * num_channels,
                  audio_file_header_info->audio_format_info.ByteSizePerFrame());
  }
  return true;
}

bool AudioWavFileHeader::WriteHeader(
    std::function<size_t(const void*, size_t)> write_func,
    const AudioFormatInfo& format_info, size_t audio_frame_size) {
  RTC_DCHECK(write_func);
  uint8_t wav_header[webrtc::kIeeeFloatWavHeaderSize] = {0};
  size_t head_size = sizeof(wav_header);
  webrtc::WriteWavHeader(
      format_info.GetChannelsCount(), format_info.GetSampleRateToInt(),
      util::ArrayFind(kCodecToWavFormat, format_info.GetAudioSampleFormat(),
                      webrtc::WavFormat::kWavFormatPcm),
      audio_frame_size, wav_header, &head_size);
  write_func(wav_header, head_size);
  return true;
}
}  // namespace core
