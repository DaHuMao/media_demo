
#include "src/core/audio/audio_device.h"

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "core/audio/audio_frame.h"
#include "core/audio/audio_frame_util.h"
#include "src/core/audio/audio_io_define.h"
#include "src/core/audio/file/file_io_creator.h"
namespace core {

class RecordingEventCallBackImpl : public RecordingEventCallBack {
 public:
  void OnRecordingStartResult(bool is_success) override {
    std::cout << "Recording start result: " << is_success << std::endl;
  }
  void OnRecordingStop() override {
    std::cout << "Recording stop" << std::endl;
  }
  void OnFirstAudioFrame() override {
    std::cout << "First audio frame" << std::endl;
  }
};

class PlayoutEventCallbackImpl : public PlayoutEventCallback {
 public:
  PlayoutEventCallbackImpl(int all_source_count,
                           std::function<void()> on_all_source_remove)
      : all_source_count_(all_source_count),
        on_all_source_remove_(on_all_source_remove) {}
  void OnPlayoutStartResult(bool is_success) override {
    std::cout << "Playout start result: " << is_success << std::endl;
  }
  void OnPlayoutStop() override { std::cout << "Playout stop" << std::endl; }
  void OnFirstAudioFrame() override {
    std::cout << "First audio frame" << std::endl;
  }
  void OnAudioSourceAutoRemoved(core::AudioRawSource* source) override {
    --all_source_count_;
    std::cout << "Audio source auto removed" << source
              << " all source count: " << all_source_count_ << std::endl;
    if (all_source_count_ == 0) {
      std::cout << "All source removed" << std::endl;
      if (on_all_source_remove_) {
        on_all_source_remove_();
      }
    }
  }

 private:
  int all_source_count_ = 0;
  std::function<void()> on_all_source_remove_;
};

void AudioDeviceTest(const std::vector<std::string>& play_file,
                     const std::string& record_file,
                     const std::string& playout_sink) {
  std::vector<std::unique_ptr<AudioRawSource>> sources;
  for (auto& file : play_file) {
    sources.push_back(core::AudioIOCreator::CreateWavFileSource(file));
  }
  std::unique_ptr<AudioRawSink> sink =
      core::AudioIOCreator::CreateWavFileWriter(
          record_file,
          AudioDeviceWrapper::GetInstance().GetDefaultRecordAudioFormatInfo());
  std::unique_ptr<AudioRawSink> playout_sink_ =
      core::AudioIOCreator::CreateWavFileWriter(
          playout_sink,
          AudioDeviceWrapper::GetInstance().GetDefaultPlayoutAudioFormatInfo());
  std::mutex mutex;
  std::condition_variable cv;
  auto record_event = RecordingEventCallBackImpl();
  auto playout_event = PlayoutEventCallbackImpl(sources.size(), [&] {
    std::unique_lock<std::mutex> lock(mutex);
    cv.notify_one();
  });
  AudioDeviceWrapper::GetInstance().AddRecordingEventSubscriber(&record_event);
  AudioDeviceWrapper::GetInstance().AddPlayoutEventSubscriber(&playout_event);
  AudioDeviceWrapper::GetInstance().AddPlayoutSink(playout_sink_.get());
  for (auto& source : sources) {
    AudioDeviceWrapper::GetInstance().AddPlayoutSource(source.get());
  }
  AudioDeviceWrapper::GetInstance().AddRecordSink(sink.get());
  std::unique_lock<std::mutex> lock(mutex);
  cv.wait(lock);
  AudioDeviceWrapper::GetInstance().RemoveRecordSink(sink.get());
  AudioDeviceWrapper::GetInstance().RemovePlayoutEventSubscriber(&playout_event);
  AudioDeviceWrapper::GetInstance().RemoveRecordingEventSubscriber(&record_event);
  for (auto& source : sources) {
    AudioDeviceWrapper::GetInstance().RemovePlayoutSource(source.get());
  }
  AudioDeviceWrapper::GetInstance().RemovePlayoutSink(playout_sink_.get());
  sink->WriteCompletion();
}


void CreateNumberAudioFile(const std::string& number_file_in,
    const std::string& mix_file_in,
    const std::string& file_out) {
  auto number_file_source = core::AudioIOCreator::CreateWavFileSource(number_file_in);
  auto mix_file_source = core::AudioIOCreator::CreateWavFileSource(mix_file_in);
  auto format = number_file_source->GetAudioFormatInfo();
  auto file_sink = core::AudioIOCreator::CreateWavFileWriter(
      file_out, format);
  std::vector<std::vector<uint8_t>> number_data(10);
  core::AudioFrameLite frame(20_ms, format);
  frame.ResetReadableSizeInByte(frame.CapacityInByte());
  int8_t* data = reinterpret_cast<int8_t*>(frame.MutableData());
  size_t data_size = frame.ByteSize();
  size_t i = 0;
  bool is_start = false;
  while(i < number_data.size()) {
    auto read_size = number_file_source->Read(data, data_size);
    if (read_size == 0) {
      break;
    }
    float energe = core::audio_util::GetAudioFrameEnerge(frame);
    constexpr float kEnergeThreshold = 0.0005;
    if (is_start) {
      if (energe < kEnergeThreshold) {
        core::audio_util::Ramp(1.0f, 0.0f, frame);
      }
      number_data[i].insert(number_data[i].end(), data, data + data_size);
      if (energe < kEnergeThreshold) {
        ++i;
        is_start = false;
      }
    } else {
      if (energe > kEnergeThreshold) {
        if (!is_start) {
          is_start = true;
          core::audio_util::Ramp(0.0f, 1.0f, frame);
          number_data[i].insert(number_data[i].end(), data, data + data_size);
        }
      }
    }
  }
  i = 0;
  core::AudioFrameLite mix_frame(1_sec, format);
  mix_frame.ResetReadableSizeInByte(mix_frame.CapacityInByte());
  auto data_mix = reinterpret_cast<int16_t*>(mix_frame.MutableData());
  while (true) {
    auto read_size = mix_file_source->Read(data_mix, mix_frame.ByteSize());
    if (read_size == 0) {
      break;
    }
    auto number_ptr = reinterpret_cast<int16_t*>(number_data[i].data());
    auto number_size = number_data[i].size() / sizeof(int16_t);
    for (size_t j = 0; j < number_size; ++j) {
      data_mix[j] += number_ptr[j];
    }
    i = (i + 1) % number_data.size();
    file_sink->Write(mix_frame.Data(), mix_frame.ByteSize());
  }
  file_sink->WriteCompletion();
}

}  // namespace core
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
int main(int argc, char const* argv[]) {
  std::string test_dir = "/Users/ztx/Desktop/test_av/";
  std::vector<std::string> play_file = {
      test_dir + "test1.wav",
      test_dir + "test.wav",
  };
  core::CreateNumberAudioFile(test_dir + "number_audio.wav", test_dir + "mix_in.wav",
      test_dir + "mix_out.wav");
  return 0;
  std::string record_file = test_dir + "record.wav";
  std::string play_sink = test_dir + "play_sink.wav";
  auto record_device_list =
      core::AudioDeviceWrapper::GetInstance().GetRecordDeviceList();
  auto playout_device_list =
      core::AudioDeviceWrapper::GetInstance().GetPlayoutDeviceList();
  for (auto& device : record_device_list) {
    std::cout << "Record device: " << device.first << " " << device.second
              << std::endl;
  }
  for (auto& device : playout_device_list) {
    std::cout << "Playout device: " << device.first << " " << device.second
              << std::endl;
  }
  core::AudioDeviceTest(play_file, record_file, play_sink);
  return 0;
}
#pragma clang diagnostic pop
