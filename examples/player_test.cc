#include "player/player.h"

#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "third_party/glfw/include/GLFW/glfw3.h"

#include "webrtc/api/task_queue/default_task_queue_factory.h"

#include "api/units/time_delta.h"
#include "core/video_render/opengl/glfw_windows.h"
#include "rtc_base/thread.h"
namespace media_demo {
std::mutex mutex_;
std::condition_variable cond;
bool is_exit = false;

class PlayerCallbackImpl : public PlayerCallback {
 public:
  void OnPrepared() override { std::cout << "OnPrepared" << std::endl; }

  void OnCompleted() override {
    std::cout << "OnCompleted" << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    cond.notify_all();
    is_exit = true;
  }

  void OnError(int32_t error_code, const std::string& error_msg) override {
    std::cout << "OnError: " << error_msg << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    cond.notify_all();
  }

  void OnBufferingStart() override {
    std::cout << "OnBufferingStart" << std::endl;
  }

  void OnBufferingEnd() override { std::cout << "OnBufferingEnd" << std::endl; }

  void OnSeekCompleted(int64_t target_ms, int64_t seek_id) override {
    std::cout << "OnSeekCompleted" << std::endl;
  }
};
class GlfwWindowsEventImpl : public core::GlfwWindowsEvent {
 public:
  GlfwWindowsEventImpl(core::GlfwWindows* windows, Player* player)
      : windows_(windows), player_(player) {}
  void OnKeyEvent(int key, int scancode, int action, int mode) override {
    std::cout << "OnKeyEvent: " << key << " action: " << action << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      windows_->Exit();
      is_exit = true;
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
      if (player_->IsPlaying()) {
        player_->Pause();
      } else {
        player_->Play();
      }
    } else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
      player_->Seek(-3000);
    } else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
      player_->Seek(3000);
    }
  }

  void OnMouseMoveEvent(double xpos, double ypos) override {}

  void OnScrollEvent(double xoffset, double yoffset) override {
    std::cout << "OnScrollEvent" << std::endl;
  }

 private:
  core::GlfwWindows* windows_ = nullptr;
  Player* player_ = nullptr;
};

void PlayerTest(const std::string& url) {
  auto player =
      Player::Create(
                     webrtc::CreateDefaultTaskQueueFactory());
  Player::PlayComponent component;
  component.video_render = util::PointDelegate<core::VideoRender>(
    core::VideoRenderFactory::CreateDefaultFactory()->Create());
  auto callback = std::make_unique<PlayerCallbackImpl>();
  core::GlfwWindows* windows = new core::GlfwWindows();
  auto event_callback = GlfwWindowsEventImpl(windows, player.get());
  core::VideoSize windows_size(800, 800);
  windows->Init(windows_size.width, windows_size.height, &event_callback);
  Player::Config config;
  config.url = url;
  config.callback = callback.get();
  config.disable_audio = false;
  config.disable_video = false;
  config.window = windows;
  config.window_size = windows_size;
  player->Init(config, std::move(component));
  player->Play();
  windows->Join([]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    return !is_exit;
  });
  player->UnInit();
  windows->Destroy();
  delete windows;
}

}  // namespace media_demo

int main(int argc, char** argv) {
  auto task_queue = rtc::Thread::Create();
  task_queue->Start();
  task_queue->PostTask([] { std::cout << "test1 test" << std::endl; });
  task_queue->PostDelayedTask([] { std::cout << "test test" << std::endl; },
                              webrtc::TimeDelta::Millis(1));
  task_queue->PostTask([] { std::cout << "test2 test" << std::endl; });
  task_queue->Stop();
  task_queue.reset();
  // media_demo::RenderTest();
  //media_demo::PlayerTest("/Users/ztx/Desktop/test_av/test_10s.mp4");
  media_demo::PlayerTest("/Users/ztx/Desktop/test_av/test_new.mp4");
  return 0;
}
