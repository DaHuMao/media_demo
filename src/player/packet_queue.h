#ifndef PLAYER_PACKET_QUEUE_H_
#define PLAYER_PACKET_QUEUE_H_
#include <vector>
#include "core/ffmpeg/common_define.h"
#include "player/packet_info.h"
#include "util/time_to_class.h"
namespace media_demo {
class PacketQueue final {
 public:
  // 缓存当前时间的前后多少毫秒的数据
  PacketQueue(util::MillisecondsClass reserved_time, bool is_video);
  ~PacketQueue();
  void PushPacket(PacketInfo& packet);
  int64_t GetDuration();
  const PacketInfo& PeekPacket();
  core::FfmpegStatus GetPacket(PacketProxy* packet);
  // 如果返回true表示不需要重新清空队列
  bool SeekTo(int64_t target_ms, uint32_t serial);
  void SetEof() {
    is_eof_ = true;
  }
  void Clear();
 private:
  void ReSizeQueue();
  void HandleReservedPacket(int index);
  int64_t PacketDiffMs(int index1, int index2);
  void UnRefPacket(int start, int end);
  /*
   * first_reserved_index_---------------read_index_---------------write_index_
   *                      |-- reserved_ms_ ---|          |---post_ms_---|
   * read_index_ 永远指向当前可读的位置，否则指向-1
   * write_index_ 永远指向当前可写的位置,否则指向-1
   * first_reserved_index_ 指向最早的可以回退的位置，否则指向-1
   *      如果是视频，first_reserved_index_指向的packet一定是关键帧
   */
  bool is_video_ = false;
  bool is_eof_ = false;
  int first_reserved_index_ = -1;
  int second_idr_index_ = -1;
  int read_index_ = -1;
  int last_writed_index_ = -1;
  int write_index_ = 0;
  util::MillisecondsClass reserved_time_;
  std::vector<PacketInfo> packet_queue_;
};
} // media_demo
#endif // PLAYER_PACKET_QUEUE_H_
