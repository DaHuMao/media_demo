#ifndef WEBRTC_AUDIO_AUDIO_FRAME_H_
#define WEBRTC_AUDIO_AUDIO_FRAME_H_
#include <cstdint>
#include <vector>

#include "core/audio/audio_format_define.h"
#include "util/time_to_class.h"
namespace core {

class COMMON_DLLEXPORT AudioFormatSizeView {
 public:
  AudioFormatSizeView(size_t byte_size, const AudioFormatInfo *info);
  virtual ~AudioFormatSizeView() = default;
  size_t ByteSize() const { return byte_size_; }
  size_t SampleSize() const {
    return byte_size_ / audio_format_ptr_->ByteSizePerSample();
  }
  size_t OneDimByteSize() const {
    return audio_format_ptr_->IsPlanner()
               ? byte_size_ / audio_format_ptr_->GetChannelsCount()
               : byte_size_;
  }
  size_t FrameSize() const {
    return byte_size_ / audio_format_ptr_->ByteSizePerFrame();
  }
  util::MillisecondsClass SizeInMs() const {
    return util::MillisecondsClass(
        audio_format_ptr_->AudioByteSizeToMs(byte_size_));
  }
  size_t FormatConvertByteSize(const AudioFormatInfo &tar_format) const {
    // In case the calculated ByteSize is not an integer multiple of the
    // AudioFrame, and we need to make sure that ByteSize is an integer multiple
    // of FrameSize
    return FormatConvertFrameSize(tar_format) * tar_format.ByteSizePerFrame();
  }
  size_t FormatConvertSampleSize(const AudioFormatInfo &tar_format) const {
    return FormatConvertFrameSize(tar_format) * tar_format.GetChannelsCount();
  }
  size_t FormatConvertFrameSize(const AudioFormatInfo &tar_format) const {
    return tar_format.AudioConvertByteSize(AudioFormat(), ByteSize()) /
           tar_format.ByteSizePerFrame();
  }
  const AudioFormatInfo &AudioFormat() const { return *audio_format_ptr_; }

 protected:
  AudioFormatSizeView() = default;
  size_t byte_size_ = 0;
  const AudioFormatInfo *audio_format_ptr_ = nullptr;
};

class COMMON_DLLEXPORT AudioFrameLiteView : public AudioFormatSizeView {
 public:
  AudioFrameLiteView(const void *data, size_t size,
                     const AudioFormatInfo *info);
  ~AudioFrameLiteView() override = default;
  const void *Data() const { return const_data_; }
  size_t CopyTo(void *data, size_t data_capacity_in_byte) const;
  virtual std::vector<const uint8_t *> PlanarData() const;

 protected:
  AudioFrameLiteView() = default;
  const void *const_data_ = nullptr;
};

// 如果音频数据是planar格式，那么data的每个元素是一个channel的数据,数组维度是channel的数量
// 如果音频数据是packed格式，那么data的每个元素是一个frame的数据, 数组维度是1
class COMMON_DLLEXPORT AudioFrameMaybePlanarView final : public AudioFrameLiteView {
 public:
  AudioFrameMaybePlanarView(const AudioFrameMaybePlanarView&);
  AudioFrameMaybePlanarView(AudioFrameMaybePlanarView&&) noexcept;
  AudioFrameMaybePlanarView(const std::vector<const uint8_t *> &data,
                       size_t one_dim_size, const AudioFormatInfo& info);
  ~AudioFrameMaybePlanarView() override = default;
  std::vector<const uint8_t *> PlanarData() const override { return const_data_arr_; }

 private:
  AudioFrameMaybePlanarView& operator=(const AudioFrameMaybePlanarView&) = delete;
  const std::vector<const uint8_t *> const_data_arr_;
  AudioFormatInfo audio_format_;
};

class COMMON_DLLEXPORT AudioFrameLiteDelegate : public AudioFrameLiteView {
 public:
  AudioFrameLiteDelegate(uint8_t *data, size_t readable_data_size,
                         size_t capacity_in_byte,
                         const AudioFormatInfo &audio_format);
  AudioFrameLiteDelegate(uint8_t *data,
                         util::MillisecondsClass readable_data_size,
                         size_t capacity_in_byte,
                         const AudioFormatInfo &audio_format);
  AudioFrameLiteDelegate(uint8_t *data, size_t readable_data_size,
                         size_t capacity_in_byte, int sample_rate_hz,
                         int num_channel);
  AudioFrameLiteDelegate(AudioFrameLiteDelegate&& src) noexcept;
  AudioFrameLiteDelegate(const AudioFrameLiteDelegate &) noexcept;
  ~AudioFrameLiteDelegate() override = default;
  // 默认从data的0位置开始写入，
  // 如果data_size_in_byte大于capacity_in_byte_，
  // 则只写入capacity_in_byte_大小的数据
  size_t CopyFrom(const void *data, size_t data_size_in_byte);
  size_t CopyFrom(const AudioFrameLiteView &audio_frame);
  size_t CopyFrom(const AudioFrameMaybePlanarView &audio_frame);
  uint8_t *MutableData() { return data_; }
  std::vector<uint8_t *> MutablePlanarData();
  size_t CapacityInByte() const { return capacity_in_byte_; }
  size_t CapacityInSamples() const {
    return capacity_in_byte_ / audio_format_ptr_->ByteSizePerSample();
  }
  size_t CapacityInFrames() const {
    return capacity_in_byte_ / audio_format_ptr_->ByteSizePerFrame();
  }
  util::MillisecondsClass CapacityInMs() const {
    return util::MillisecondsClass(
        audio_format_ptr_->AudioByteSizeToMs(capacity_in_byte_));
  }
  void ResetReadableSizeInByte(size_t size);
  void ResetReadableSize(util::MillisecondsClass ms);
  AudioFormatInfo &MutableAudioFormat() { return audio_format_; }
  uint8_t *CurrentData() { return data_ + byte_size_; }
  size_t FreeSizeInByte() const { return capacity_in_byte_ - byte_size_; }
  size_t FreeSizeInSamples() const {
    return FreeSizeInByte() / audio_format_ptr_->ByteSizePerSample();
  }
  size_t FreeSizeInFrames() const {
    return FreeSizeInByte() / audio_format_ptr_->ByteSizePerFrame();
  }
  // 从当前数据的末尾开始写入数据，如果data_size_in_byte大于剩余空间，
  // 则只写入剩余空间大小的数据
  size_t Append(const void *data, size_t data_size_in_byte);

 protected:
  AudioFrameLiteDelegate() = default;
  AudioFrameLiteDelegate(const AudioFormatInfo &audio_format);
  uint8_t *data_ = nullptr;
  size_t capacity_in_byte_ = 0;
  AudioFormatInfo audio_format_;
};

class COMMON_DLLEXPORT AudioFrameLite final : public AudioFrameLiteDelegate {
 public:
  AudioFrameLite(const AudioFormatInfo &info);
  AudioFrameLite(util::MillisecondsClass capacity_in_ms,
                 const AudioFormatInfo &info);
  AudioFrameLite(util::MillisecondsClass capacity_in_ms, int sample_rate_hz,
                 int num_channel);
  AudioFrameLite(const AudioFormatInfo &info, size_t capacity_in_byte,
                 size_t readable_size_in_byte = 0);
  AudioFrameLite(const AudioFormatInfo &info,
                 util::MillisecondsClass capacity_in_ms,
                 util::MillisecondsClass readable_size_in_ms = 0_ms);
  AudioFrameLite(AudioFrameLite &&src);
  AudioFrameLite &operator=(AudioFrameLite &&src);
  ~AudioFrameLite() override;
  AudioFrameLite(const AudioFrameLite &) = delete;
  AudioFrameLite &operator=(const AudioFrameLite &) = delete;
  void ExpandCapacityIfNeed(size_t capacity_in_byte);
};

}  // namespace core
#endif  // WEBRTC_AUDIO_AUDIO_FRAME_H_
