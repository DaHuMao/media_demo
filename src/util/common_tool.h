/**
* common_tool.h, December 2020.
*
* Copyright 2020 fenbi.com. All rights reserved.
* FENBI.COM PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
*/
#ifndef WEBRTC_AUDIO_UTILITY_COMMON_TOOL_H_
#define WEBRTC_AUDIO_UTILITY_COMMON_TOOL_H_

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

#include "webrtc/system_wrappers/include/thread_wrapper.h"
#include "webrtc/common_audio/ring_buffer.h"

namespace webrtc {

//----------------------  CacheBuffer ----------------------------------//
constexpr size_t kMaxBufferSize = 50 * 1024 * 1024;
class RingBufferWrapper {
public:
  RingBufferWrapper(const RingBufferWrapper&) = delete;
  RingBufferWrapper(RingBufferWrapper&&) = delete;
  // If expandable_ is true, the buffer will automatically expand when it is full,
  // otherwise, the buffer will overwrite the past data when it is full
  RingBufferWrapper(size_t capacity, bool auto_expand_capacity = false, size_t max_buffer_capacity = kMaxBufferSize);
  virtual ~RingBufferWrapper();
  const void* BufferRead(void* data, size_t in_size, size_t* out_size);
  size_t BufferRead(void* data, size_t size);
  size_t BufferWrite(const void* data, size_t size);
  void Clear();
  size_t BufferCurrentSize();
  size_t BufferCapacity();
  int BufferSeek(int offset);
  void AdjustCapacity(size_t capacity);

private:
  size_t CalculatedExpansionCapacity(size_t write_size);
private:
  RingBuffer* buff_handle_ = nullptr;
  // If buffer is in expandable mode, exceeding this capacity will automatically turn into non-expandable mode
  bool auto_adjust_capacity_ = false;
  size_t max_buffer_capacity_ = kMaxBufferSize;
};
//----------------------  CacheBuffer ----------------------------------//
class ThreadSafeRingBufferWrapper {
public:
  ThreadSafeRingBufferWrapper(size_t capacity, bool auto_expand_capacity = false,
      size_t max_buffer_capacity = kMaxBufferSize);
  virtual ~ThreadSafeRingBufferWrapper();
  const void* BufferRead(void* data, size_t in_size, size_t* out_size);
  size_t BufferRead(void* data, size_t size);
  size_t BufferWrite(const void* data, size_t size);
  size_t BufferCurrentSize();
  size_t SizeCanBeWrite();
  void Clear();
private:
  std::unique_ptr<RingBufferWrapper> ring_buffer_wrapper_;
  // TODO:更换成读写锁
  std::mutex mutex_;
};
//----------------------  ThreadHandle ----------------------------------//
class ThreadHandle {
public:
  ThreadHandle(webrtc::ThreadPriority thread_priority = webrtc::kNormalPriority,
      const char* thread_name = "ThreadHandle");
  virtual ~ThreadHandle();
  virtual bool Start();
  virtual bool Stop();
  virtual bool IsRuning();

  static std::unique_ptr<ThreadHandle> CreateThread(std::function<bool(void)> handle_func, const char* thread_name = "ThreadHandle",
      webrtc::ThreadPriority thread_priority = webrtc::kNormalPriority);

protected:
  virtual bool Handle() = 0;
  virtual void FuctionIsOver();
  static bool RunThread(ThreadObj obj);
protected:
  std::unique_ptr<webrtc::ThreadWrapper> thread_handle_;
  /*
   * <is_running_> indicates whether the thread function is running
   * <is_stop_> indicates whether the thread has been called Stop() function
   * This means that <is_stop_> is not necessarily TRUE if is_running_ is FALSE, and <is_running_> is FALSE
   * may be due to thread function exit, but it does not mean that the Stop() function has been called,
   * because calling the Stop() function may be accompanied by some resource release.
   * On the contrary, <is_stop_> is TRUE, <is_running_> must be FALSE
   * */
  std::atomic<bool> is_stop_;
  std::atomic<bool> is_running_;
};
//----------------------  ThreadHandle ----------------------------------//

} // namespace fenbi
#endif // WEBRTC_AUDIO_UTILITY_COMMON_TOOL_H_
