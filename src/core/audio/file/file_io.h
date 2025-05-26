/**
 * file_io.h, 2023.
 * Create by zhangtongxiao
 */
#ifndef WEBRTC_FILE_READER_AUDIO_FILE_IO_H_
#define WEBRTC_FILE_READER_AUDIO_FILE_IO_H_
#include "core/audio/file/audio_file_header.h"
#ifdef WEBRTC_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "core/audio/audio_io_define.h"
namespace core {
class AudioPcmFileSource : public AudioFileSource {
 public:
  // raw bin
  AudioPcmFileSource(const std::string &file_name,
                     const AudioFormatInfo &audio_format);
  // wav format
  AudioPcmFileSource(const std::string &file_name,
                     std::unique_ptr<AudioFileHeader> header);
  virtual ~AudioPcmFileSource();
  size_t Read(void *data_ptr, size_t data_size) override;
  int DiscardDataSizeInByte(size_t size) override;
  size_t CurrentSize() override;
  util::MillisecondsClass CurrentSizeMs() override;
  SourceStatus GetSourceStatus() const override;
  const AudioFormatInfo &GetAudioFormatInfo() const override;
  int FillZeroFront(util::MillisecondsClass time_ms) override;
  void SetCyclicMode(bool enable) override { enable_cyclic_mode_ = enable; }
  int Seek(int offset) override;

 private:
  int ReadWithSeek(char **read_ptr, size_t data_size, int *should_seek_size);
  void OpenFile(const std::string &file_name);
  size_t CurrentSizeInternal();
  AudioFormatInfo audio_format_;
  SourceStatus status_ = SourceStatus::kError;
  int should_seek_size_ = 0;
  FILE *fp_ = nullptr;
  size_t audio_samples_in_bytes_;
  size_t fix_offset_ = 0;
  bool enable_cyclic_mode_ = false;
};
class FileWriter final : public RawSink {
 public:
  FileWriter(const std::string &file_name);
  virtual ~FileWriter();
  void WriteCompletion() override;
  size_t Write(const void *data_ptr, size_t size) override;
  int MoveFilePtr(int offset);
  size_t CurrentPtrOffsetFromStartPos();

 private:
  FILE *fp_ = nullptr;
};
class FileWriterWithHeader final : public AudioRawSink {
 public:
  FileWriterWithHeader(const std::string &file_name,
                       const AudioFormatInfo &format_info,
                       std::unique_ptr<AudioFileHeader> header);
  ~FileWriterWithHeader() override;
  void WriteCompletion() override;
  size_t Write(const void *data_ptr, size_t size) override;
  const AudioFormatInfo &GetNeededAudioFormatInfo() const override;

 private:
  void WriteWavHeader(size_t audio_len);
  std::unique_ptr<FileWriter> file_writer_;
  AudioFormatInfo format_info_;
  std::unique_ptr<AudioFileHeader> header_;
  size_t fix_header_size_ = 0;
};
}  // namespace core
#endif  // WEBRTC_FILE_READER_AUDIO_FILE_IO_H_
