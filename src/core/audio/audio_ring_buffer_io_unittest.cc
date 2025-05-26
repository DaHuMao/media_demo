#include "core/audio/audio_ring_buffer_io.h"

#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "common_audio/signal_processing/include/signal_processing_library.h"
#include "gtest/gtest.h"
#include "util/time_to_class.h"

namespace core {
constexpr AudioFormatInfo g_info = k16kMonoPcm16;
constexpr int size_int = 100;
constexpr int size_byte = size_int * sizeof(int);
class AudioRingBufferIoTest : public ::testing::Test {
 protected:
  AudioRingBufferIoTest() { data_tmp.resize(size_int); }
  void SetUp() override {}
  void TearDown() override {}
  std::vector<int> data_tmp;
};

std::unique_ptr<AudioRingBufferIo> CreateAudioRingBufferIo(size_t capacity) {
  return AudioRingBufferIo::Create(g_info, capacity);
}

std::vector<int> GenerateData(int size) {
  std::vector<int> data(size);
  for (int i = 0; i < size; ++i) {
    data[i] = i;
  }
  return data;
}

// 测试通用边界
TEST_F(AudioRingBufferIoTest, TestCommonBoundary) {
  auto buffer = CreateAudioRingBufferIo(size_byte);
  auto data = GenerateData(size_int);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte);
  EXPECT_EQ(buffer->CurrentSize(), 0);
  auto read_size = buffer->Read(data_tmp.data(), size_byte);
  EXPECT_EQ(read_size, 0);
  auto size = buffer->Write(data.data(), 0);
  EXPECT_EQ(size, 0);
  size = buffer->Write(nullptr, size_byte);
  EXPECT_EQ(size, 0);
  buffer->WriteCompletion();
  buffer->Reset();
  buffer->Write(data.data(), size_byte);
  size = buffer->Read(nullptr, size_byte);
  EXPECT_EQ(size, 0);
  EXPECT_DEATH(buffer->Write(data_tmp.data(), size_byte + 1), "");
  size = buffer->DiscardDataSizeInByte(size_byte + 1);
  EXPECT_EQ(size, size_byte + 1);
  buffer->Write(data.data(), size_byte);
  size = buffer->Read(data_tmp.data(), size_byte + 2);
  EXPECT_EQ(size, size_byte - 1);
}

TEST_F(AudioRingBufferIoTest, TestAudioFormat) {
  auto buffer = CreateAudioRingBufferIo(100);
  EXPECT_EQ(buffer->GetAudioFormatInfo(), g_info);
  EXPECT_EQ(buffer->GetNeededAudioFormatInfo(), g_info);
}

TEST_F(AudioRingBufferIoTest, TestReadWrite) {
  auto buffer = CreateAudioRingBufferIo(size_byte);
  auto data = GenerateData(size_int);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte);
  EXPECT_EQ(buffer->CurrentSize(), 0);
  buffer->Write(data.data(), size_byte / 2);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte / 2);
  EXPECT_EQ(buffer->CurrentSize(), size_byte / 2);
  buffer->Read(data_tmp.data(), size_byte / 2);
  for (int i = 0; i < size_int / 2; ++i) {
    EXPECT_EQ(data_tmp[i], data[i]);
  }
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte);
  EXPECT_EQ(buffer->CurrentSize(), 0);
}

TEST_F(AudioRingBufferIoTest, TestOverwrite) {
  auto buffer = CreateAudioRingBufferIo(size_byte);
  auto data = GenerateData(size_int * 2);
  buffer->Write(data.data(), size_byte);
  buffer->Write(data.data() + size_int, size_byte);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), 0);
  EXPECT_EQ(buffer->CurrentSize(), size_byte);
  auto size = buffer->Write(data.data(), size_byte / 2);
  EXPECT_EQ(size, size_byte / 2);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), 0);
  EXPECT_EQ(buffer->CurrentSize(), size_byte);
  size = buffer->Read(data_tmp.data(), size_byte / 2);
  EXPECT_EQ(size, size_byte / 2);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte / 2);
  EXPECT_EQ(buffer->CurrentSize(), size_byte / 2);
  for (int i = 0; i < size_int / 2; ++i) {
    EXPECT_EQ(data_tmp[i], data[i + size_int + size_int / 2]);
  }
  size = buffer->Read(data_tmp.data(), size_byte);
  EXPECT_EQ(size, size_byte / 2);
  for (int i = 0; i < size_int / 2; ++i) {
    EXPECT_EQ(data_tmp[i], data[i]);
  }
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte);
  EXPECT_EQ(buffer->CurrentSize(), 0);
}

TEST_F(AudioRingBufferIoTest, TestSourceStatusAndReset) {
  auto buffer = CreateAudioRingBufferIo(100);
  EXPECT_EQ(buffer->GetSourceStatus(), SourceStatus::kStreaming);
  buffer->WriteCompletion();
  EXPECT_EQ(buffer->GetSourceStatus(), SourceStatus::kStatic);
  auto data = GenerateData(100);
  auto size = buffer->Write(data.data(), 100);
  EXPECT_EQ(size, 0);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), 0);
  EXPECT_EQ(buffer->CurrentSize(), 0);
  buffer->Reset();
  EXPECT_EQ(buffer->GetSourceStatus(), SourceStatus::kStreaming);
  size = buffer->Write(data.data(), 100);
  EXPECT_EQ(size, 100);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), 0);
  EXPECT_EQ(buffer->CurrentSize(), 100);
}

TEST_F(AudioRingBufferIoTest, TestDiscardDataSizeInByte) {
  auto buffer = CreateAudioRingBufferIo(size_byte);
  auto data = GenerateData(size_int);
  buffer->Write(data.data(), size_byte);
  buffer->DiscardDataSizeInByte(size_byte / 2);
  EXPECT_EQ(buffer->CurrentSize(), size_byte / 2);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte / 2);
}

TEST_F(AudioRingBufferIoTest, TestFillZeroFront) {
  auto buffer = CreateAudioRingBufferIo(size_byte);
  auto data = GenerateData(size_int);
  buffer->FillZeroFront(1_ms);
  auto zero_size = g_info.AudioMsToByteSize(1_ms);
  EXPECT_EQ(buffer->CurrentSize(), zero_size);
  auto size = buffer->Write(data.data(), size_byte / 2);
  EXPECT_EQ(size, size_byte / 2);
  EXPECT_EQ(buffer->CurrentSize(), size_byte / 2 + zero_size);
  size = buffer->Read(data_tmp.data(), size_byte / 2 + zero_size);
  EXPECT_EQ(size, size_byte / 2 + zero_size);
  auto zero_size_int = zero_size / sizeof(int);
  for (int i = 0; i < zero_size_int; ++i) {
    EXPECT_EQ(data_tmp[i], 0);
  }
  for (int i = 0; i < size_int / 2; ++i) {
    EXPECT_EQ(data_tmp[i + zero_size_int], data[i]);
  }
  buffer->Write(data.data(), size_byte / 2);
  buffer->FillZeroFront(1_ms);
  EXPECT_EQ(buffer->CurrentSize(), size_byte / 2 + zero_size);
  buffer->Read(data_tmp.data(), size_byte / 2 + zero_size);
  for (int i = 0; i < zero_size_int; ++i) {
    EXPECT_EQ(data_tmp[i], 0);
  }
  for (int i = 0; i < size_int / 2; ++i) {
    EXPECT_EQ(data_tmp[i + zero_size_int], data[i]);
  }
}

TEST_F(AudioRingBufferIoTest, TestCurrentSizeInMs) {
  auto size_byte = g_info.AudioMsToByteSize(10_ms);
  auto buffer = CreateAudioRingBufferIo(size_byte);
  EXPECT_EQ(buffer->CurrentSizeMs(), 0_ms);
  auto data = GenerateData(size_byte / sizeof(int));
  buffer->Write(data.data(), size_byte);
  EXPECT_EQ(buffer->CurrentSizeMs(), 10_ms);
}

TEST_F(AudioRingBufferIoTest, TestResetUseableCapacity) {
  auto buffer = CreateAudioRingBufferIo(size_byte);
  auto data = GenerateData(size_int);
  buffer->Write(data.data(), size_byte);
  buffer->ResetUseableCapacity(size_byte / 2);
  EXPECT_EQ(buffer->CurrentSize(), size_byte);
  auto size = buffer->Read(data_tmp.data(), size_byte / 2);
  EXPECT_EQ(size, size_byte / 2);
  for (int i = 0; i < size_int / 2; ++i) {
    EXPECT_EQ(data_tmp[i], data[i]);
  }
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), 0);
  size = buffer->Read(data_tmp.data(), size_byte / 2);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), size_byte / 2);
  buffer->Write(data.data(), size_byte / 2);
  buffer->Write(data.data(), size_byte / 2);
  EXPECT_EQ(buffer->CurrentSize(), size_byte / 2);
  EXPECT_EQ(buffer->FreeSpaceBeforeOverwriting(), 0);
}

void BlockingWriteOrRead(bool is_read,
                         std::function<void(AudioRingBufferIo *)> func) {
  std::mutex mtx;
  std::condition_variable cv;
  std::atomic<bool> ready(false);
  AudioRingBufferIo *buffer_ptr = nullptr;

  std::thread t([&]() {
    bool is_read_tmp = is_read;
    auto buffer = CreateAudioRingBufferIo(size_byte);
    std::unique_ptr<int[]> data(new int[size_int]);
    {
      std::lock_guard<std::mutex> lock(mtx);
      buffer_ptr = buffer.get();
      ready = true;
      cv.notify_one();
    }
    if (is_read_tmp) {
      buffer->Write(data.get(), size_byte / 2);
      buffer->BlockingRead(data.get(), size_byte);
    } else {
      buffer->Write(data.get(), size_byte);
      buffer->BlockingWrite(data.get(), size_byte);
    }
  });
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return ready.load(); });
  }
  t.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  auto time_now = TimeNow();
  if (buffer_ptr) {
    func(buffer_ptr);
  }
  auto time_used = TimeNow() - time_now;
  EXPECT_LT(time_used, 500_ms);
}

void TestBlockingAndCancel(bool is_cancel_all, bool is_read) {
  BlockingWriteOrRead(is_read, [&](AudioRingBufferIo *buffer_ptr) {
    if (is_cancel_all) {
      buffer_ptr->CancelAll();
    } else {
      if (is_read) {
        buffer_ptr->CancelRead();
      } else {
        buffer_ptr->CancelWrite();
      }
    }
  });
}

TEST_F(AudioRingBufferIoTest, TestBlockingReadAndCancelRead) {
  TestBlockingAndCancel(false, true);
  TestBlockingAndCancel(true, true);
}

TEST_F(AudioRingBufferIoTest, TestBlockingWriteAndCancelWrite) {
  TestBlockingAndCancel(false, false);
  TestBlockingAndCancel(true, false);
}

TEST_F(AudioRingBufferIoTest, TestReadWakeUpBlockingWrite) {
  BlockingWriteOrRead(true, [&](AudioRingBufferIo *buffer_ptr) {
    buffer_ptr->Write(data_tmp.data(), size_byte);
  });
}

TEST_F(AudioRingBufferIoTest, TestWriteWakeUpBlockingRead) {
  BlockingWriteOrRead(false, [&](AudioRingBufferIo *buffer_ptr) {
    buffer_ptr->Read(data_tmp.data(), size_byte);
  });
}

void TestBlockReadAndWrite(int buffer_size_int, int read_one_time,
                           int write_one_time) {
  int size_byte_size = buffer_size_int * sizeof(int);
  size_t data_size = buffer_size_int * 5;
  auto data = GenerateData(data_size);
  int read_size_int = 0;
  int write_size_int = 0;
  auto buffer = CreateAudioRingBufferIo(size_byte_size);
  int read_size_one_time_byte = read_one_time * sizeof(int);
  int write_size_one_time_byte = write_one_time * sizeof(int);
  std::thread t_write([&]() {
    while (write_size_int < data_size) {
      int size = buffer->BlockingWrite(data.data() + write_size_int,
                                       write_size_one_time_byte);
      EXPECT_EQ(size, write_size_one_time_byte);
      write_size_int += write_one_time;
    }
  });
  std::vector<int> read_data(buffer_size_int);
  std::thread t_read([&]() {
    while (read_size_int < data_size) {
      int size =
          buffer->BlockingRead(read_data.data(), read_size_one_time_byte);
      EXPECT_EQ(size, read_size_one_time_byte);
      for (int i = 0; i < read_one_time; ++i) {
        EXPECT_EQ(read_data[i], data[read_size_int + i]);
      }
      read_size_int += read_one_time;
    }
  });
  t_write.join();
  t_read.join();
}

TEST_F(AudioRingBufferIoTest, TestBlockingReadAndWrite) {
  TestBlockReadAndWrite(1000, 10, 10);
  TestBlockReadAndWrite(1000, 10, 100);
  TestBlockReadAndWrite(1000, 100, 10);
  TestBlockReadAndWrite(10000, 10, 1000);
  TestBlockReadAndWrite(10000, 1000, 10);
}

}  // namespace core
