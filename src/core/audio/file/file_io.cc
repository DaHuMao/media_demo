#include "core/audio/file/file_io.h"

#include <cstdio>
#include <cstring>

#include "webrtc/rtc_base/checks.h"

#include "core/audio/audio_io_define.h"
#include "util/log.h"
namespace core {
const char *kTag = "AudioFileIO";
//-------------    AudioPcmFileSource   ---------------//
AudioPcmFileSource::AudioPcmFileSource(const std::string &file_name,
                                       const AudioFormatInfo &audio_format)
    : audio_format_(audio_format), audio_samples_in_bytes_(0) {
  OpenFile(file_name);
  if (nullptr != fp_) {
    fseek(fp_, 0L, SEEK_END);
    audio_samples_in_bytes_ = ftell(fp_);
    fseek(fp_, 0L, SEEK_SET);
    status_ = SourceStatus::kStatic;
  } else {
    LOGI_TAG(kTag) << "faild open source file: " << file_name;
  }
}

AudioPcmFileSource::AudioPcmFileSource(
    const std::string &file_name, std::unique_ptr<AudioFileHeader> header) {
  RTC_DCHECK(header);
  OpenFile(file_name);
  if (nullptr != fp_) {
    AudioFileHeaderInfo info;
    fseek(fp_, 0L, SEEK_END);
    audio_samples_in_bytes_ = ftell(fp_);
    fseek(fp_, 0L, SEEK_SET);
    header->ReadHeader(this, &info);
    fix_offset_ = ftell(fp_);
    audio_format_ = info.audio_format_info;
    RTC_DCHECK(audio_format_.ValidPcmCheck())
        << "invalid audio_format: " << audio_format_.ToString();
    if (info.audio_frame_size == 0) {
      audio_samples_in_bytes_ = audio_samples_in_bytes_ - fix_offset_;
    } else {
      audio_samples_in_bytes_ =
          info.audio_frame_size * audio_format_.ByteSizePerFrame();
    }
    status_ = SourceStatus::kStatic;
  } else {
    fclose(fp_);
    fp_ = nullptr;
    LOGE_TAG(kTag) << "faild open wav file";
  }
}

AudioPcmFileSource::~AudioPcmFileSource() {
  if (nullptr != fp_) {
    fclose(fp_);
  }
}
int AudioPcmFileSource::FillZeroFront(util::MillisecondsClass time_ms) {
  if (nullptr == fp_) {
    return 0;
  }
  if (static_cast<size_t>(ftell(fp_)) == fix_offset_) {
    should_seek_size_ +=
        0 - static_cast<int>(audio_format_.AudioMsToByteSize(time_ms));
    return static_cast<int>(time_ms.Value());
  }
  return 0;
}
size_t AudioPcmFileSource::Read(void *data_ptr, size_t data_size) {
  if (nullptr == fp_ || nullptr == data_ptr) {
    return 0;
  }
  size_t cur_size = CurrentSizeInternal();
  if (enable_cyclic_mode_ && cur_size == 0) {
    fseek(fp_, static_cast<long>(fix_offset_), SEEK_SET);
    cur_size = CurrentSizeInternal();
  }
  if (cur_size == 0) {
    return 0;
  }
  char *read_ptr = static_cast<char *>(data_ptr);
  // CurrentSizeInternal maybe changed because of should_seek_size_'s change
  size_t should_read_size =
      ReadWithSeek(&read_ptr, data_size, &should_seek_size_);
  cur_size = CurrentSizeInternal();
  size_t size_tmp = data_size - should_read_size;
  should_read_size = should_read_size < cur_size ? should_read_size : cur_size;
  size_tmp += fread(read_ptr, sizeof(char), should_read_size, fp_);
  if (size_tmp < data_size) {
    RTC_DCHECK(CurrentSizeInternal() == 0)
        << "CurrentSizeInternal: " << CurrentSizeInternal();
    size_tmp += Read(read_ptr + size_tmp, data_size - size_tmp);
  }
  return size_tmp;
}
int AudioPcmFileSource::DiscardDataSizeInByte(size_t offset) {
  if (nullptr == fp_) {
    return -1;
  }
  should_seek_size_ += static_cast<int>(offset);
  if (should_seek_size_ > 0) {
    int size_can_seek =
        std::min(static_cast<int>(audio_samples_in_bytes_ - ftell(fp_)),
                 should_seek_size_);
    should_seek_size_ = 0;
    fseek(fp_, size_can_seek, SEEK_CUR);
  }
  return 0;
}
size_t AudioPcmFileSource::CurrentSizeInternal() {
  if (nullptr != fp_) {
    auto tt = ftell(fp_);
    return static_cast<size_t>(audio_samples_in_bytes_ - tt -
                               should_seek_size_ + fix_offset_);
  }
  return 0;
}

size_t AudioPcmFileSource::CurrentSize() {
  return enable_cyclic_mode_ ? audio_samples_in_bytes_ : CurrentSizeInternal();
}

util::MillisecondsClass AudioPcmFileSource::CurrentSizeMs() {
  return audio_format_.AudioByteSizeToMs(CurrentSize());
}

SourceStatus AudioPcmFileSource::GetSourceStatus() const { return status_; }
const AudioFormatInfo &AudioPcmFileSource::GetAudioFormatInfo() const {
  return audio_format_;
}

int AudioPcmFileSource::ReadWithSeek(char **read_ptr, size_t data_size,
                                     int *should_seek_size) {
  int should_read_size = static_cast<int>(data_size);
  if (*should_seek_size < 0) {
    memset(*read_ptr, 0, data_size);
    should_read_size = static_cast<int>(data_size) + *should_seek_size;
    if (should_read_size > 0) {
      *read_ptr -= *should_seek_size;
      *should_seek_size = 0;
    } else {
      *should_seek_size += static_cast<int>(data_size);
      should_read_size = 0;
    }
  }
  return should_read_size;
}

int AudioPcmFileSource::Seek(int offset) {
  if (nullptr == fp_) {
    return -1;
  }
  if (offset < 0) {
    offset = std::max(offset, -static_cast<int>(ftell(fp_)));
  }
  return fseek(fp_, offset, SEEK_CUR);
}
void AudioPcmFileSource::OpenFile(const std::string &file_name) {
#ifdef LIVE_ENGINE_WIN
  int len = MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, nullptr, 0);
  std::wstring wstr(len, 0);
  MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, &wstr[0], len);
  fp_ = _wfopen(wstr.c_str(), L"rb");
#else
  fp_ = fopen(file_name.c_str(), "rb");
#endif
  RTC_DCHECK(nullptr != fp_) << "faild open read file: " << file_name;
}
//-------------    AudioPcmFileSource   ---------------//
//-------------    FileWriter   ---------------//
FileWriter::FileWriter(const std::string &file_name) {
  if ("" != file_name) {
#ifdef LIVE_ENGINE_WIN
    int len =
        MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, nullptr, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, &wstr[0], len);
    fp_ = _wfopen(wstr.c_str(), L"wb");
#else
    fp_ = fopen(file_name.c_str(), "wb");
#endif
    if (nullptr == fp_) {
      LOGI_TAG(kTag) << "faild open write file: " << file_name;
      RTC_DCHECK(false) << "faild open write file: " << file_name;
    }
  }
}
FileWriter::~FileWriter() {
  if (nullptr != fp_) {
    fclose(fp_);
  }
}
size_t FileWriter::Write(const void *data_ptr, size_t size) {
  if (nullptr != fp_ && nullptr != data_ptr) {
    return fwrite(data_ptr, 1, size, fp_);
  }
  return 0;
}
void FileWriter::WriteCompletion() {
  if (nullptr != fp_) {
    fclose(fp_);
    fp_ = nullptr;
  }
}
int FileWriter::MoveFilePtr(int offset) {
  if (nullptr == fp_ || offset == 0) {
    return 0;
  }
  if (offset < 0) {
    offset = std::max(offset, -static_cast<int>(ftell(fp_)));
  }
  return fseek(fp_, offset, SEEK_CUR);
}
size_t FileWriter::CurrentPtrOffsetFromStartPos() {
  return nullptr == fp_ ? 0 : ftell(fp_);
}
//-------------    FileWriter   ---------------//
//-------------    FileWriterWithHeader   ---------------//
FileWriterWithHeader::FileWriterWithHeader(
    const std::string &file_name, const AudioFormatInfo &format_info,
    std::unique_ptr<AudioFileHeader> header)
    : file_writer_(std::make_unique<FileWriter>(file_name)),
      format_info_(format_info),
      header_(std::move(header)) {
  RTC_DCHECK(nullptr != file_writer_ && nullptr != header_);
  if (nullptr != file_writer_) {
    WriteWavHeader(0);
    fix_header_size_ = file_writer_->CurrentPtrOffsetFromStartPos();
  }
}
FileWriterWithHeader::~FileWriterWithHeader() {
  if (file_writer_ != nullptr) {
    WriteCompletion();
  }
}
void FileWriterWithHeader::WriteCompletion() {
  if (file_writer_ != nullptr) {
    size_t cur_size = file_writer_->CurrentPtrOffsetFromStartPos();
    if (cur_size < fix_header_size_) {
      return;
    }
    file_writer_->MoveFilePtr(0 - static_cast<int>(cur_size));
    WriteWavHeader(cur_size - fix_header_size_);
    file_writer_->WriteCompletion();
    file_writer_ = nullptr;
  }
}
size_t FileWriterWithHeader::Write(const void *data_ptr, size_t size) {
  return nullptr == file_writer_ ? 0 : file_writer_->Write(data_ptr, size);
}
const AudioFormatInfo &FileWriterWithHeader::GetNeededAudioFormatInfo() const {
  return format_info_;
}

void FileWriterWithHeader::WriteWavHeader(size_t audio_len_bytes) {
  size_t audio_frames_len = audio_len_bytes / format_info_.ByteSizePerSample();
  RTC_DCHECK(audio_frames_len * format_info_.ByteSizePerSample() ==
             audio_len_bytes);
  header_->WriteHeader(
      [this](const void *data_ptr, size_t size) {
        return this->file_writer_->Write(data_ptr, size);
      },
      format_info_, audio_frames_len);
}
//-------------    FileWriterWithHeader   ---------------//
}  // namespace core
