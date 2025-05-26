#ifndef CORE_FFMPEG_AV_PACKET_WRAPPER_H_
#define CORE_FFMPEG_AV_PACKET_WRAPPER_H_
extern "C" {
#include "libavcodec/avcodec.h"
}
namespace core {
class AvPacketWrapper {
 public:
  AvPacketWrapper();
  AvPacketWrapper(AvPacketWrapper&& oth);
  AvPacketWrapper& operator=(AvPacketWrapper&& oth);
  virtual ~AvPacketWrapper();
  void MoveFromPaket(AvPacketWrapper& oth);
  void UnRefPacket();
  AVPacket* GetPacket() { return packet_; }
  bool IsValid();
  void SetTimeBase(AVRational time_base);
  int64_t GetPtsMs();
  int64_t GetDurationMs();
 private:
  AVPacket* packet_ = nullptr;
  AVRational time_base_ = {0, 0};
};
} // namespace core
#endif // CORE_FFMPEG_AV_PACKET_WRAPPER_H_
