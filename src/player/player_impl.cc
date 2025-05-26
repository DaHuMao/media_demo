#include "player/player_impl.h"

#include <sstream>

#include "player/packet_info.h"
#include "util/log.h"
namespace media_demo {
std::atomic<int> static_id(0);
constexpr char kLogTag[] = "PlayerImpl";
PlayerImpl::PlayerImpl(
    std::unique_ptr<core::VideoRenderFactory> video_render_factory,
    util::PointDelegate<webrtc::TaskQueueFactory> task_queue_factory)
    : id_(++static_id),
      timer_(PlayerTimerType::kAudio),
      video_render_factory_(std::move(video_render_factory)),
      task_queue_factory_(std::move(task_queue_factory)),
      av_packet_queue_(10_sec) {
  player_task_queue_ = rtc::Thread::Create();
  player_task_queue_->SetName("player" + std::to_string(id_), nullptr);
  player_task_queue_->Start();
}
PlayerImpl::~PlayerImpl() { player_task_queue_.reset(); }
int PlayerImpl::Init(const Player::Config& config, PlayComponent component) {
  player_task_queue_->PostTask(
      // mutable 为了让component_tmp可以在lambda中移动
      [config, component_tmp = std::move(component), this]() mutable {
        this->InitInternal(config, std::move(component_tmp));
      });
  return 0;
}

int PlayerImpl::Play() {
  player_task_queue_->PostTask([this]() { this->PlayInternal(); });
  return 0;
}

int PlayerImpl::Pause() {
  player_task_queue_->PostTask([this]() { this->PauseInternal(); });
  return 0;
}

bool PlayerImpl::IsPlaying() { return GetState() == PlayerState::kPlaying; }

int PlayerImpl::Seek(int64_t diff_current) {
  player_task_queue_->PostTask([diff_current, this]() {
    this->SeekInternal(diff_current + this->timer_.GetCurrentMainTime());
  });
  return 0;
}

int PlayerImpl::SeekTo(int64_t target_ms) {
  player_task_queue_->PostTask([target_ms, this]() {
    this->SeekInternal(target_ms);
  });
  return 0;
}

int PlayerImpl::UnInit() {
  player_task_queue_->BlockingCall([this] { this->UnInitInternal(); });
  player_task_queue_->Stop();
  return 0;
}

int64_t PlayerImpl::GetCurPositionMs() { return reader_info_.current_pos_ms; }

int64_t PlayerImpl::GetDurationMs() { return player_info_.duration_ms; }
// AudioDecodeToRenderCallback
void PlayerImpl::OnAudioCompleted() {
  if (config_.callback) {
    config_.callback->OnCompleted();
  }
}

void PlayerImpl::OnAudioError() { SetError(-1, "audio error"); }

core::FfmpegStatus PlayerImpl::GetAudioPacket(PacketProxy& packet) {
  auto res = core::FfmpegStatus::kEof;
  if (!config_.disable_audio && reader_info_.reader_->HasAudio()) {
    res = av_packet_queue_.GetPacket(&packet, false);
    while (res == core::FfmpegStatus::kOk) {
      if (packet.GetPtsMs() + packet.GetDurationMs() < start_time_) {
        res = av_packet_queue_.GetPacket(&packet, false);
      } else {
        break;
      }
    }
  }
  return res;
}

// VideoDecodeToRenderCallback
void PlayerImpl::OnVideoCompleted() {
  if (config_.callback) {
    config_.callback->OnCompleted();
  }
}

void PlayerImpl::OnVideoError() { SetError(-1, "video error"); }

core::FfmpegStatus PlayerImpl::GetVideoPacket(PacketProxy& packet) {
  if (!config_.disable_video && reader_info_.reader_->HasVideo()) {
    return av_packet_queue_.GetPacket(&packet, true);
  }
  return core::FfmpegStatus::kEof;
}

void PlayerImpl::InitInternal(const Config& config, PlayComponent component) {
  if (player_info_.state != PlayerState::kIdle) {
    return;
  }
  config_ = config;
  component_ = std::move(component);
  if (config_.url.empty() || config_.window == nullptr ||
      config_.window_size.width == 0 || config_.window_size.height == 0) {
    SetError(-1, "url is empty or window is nullptr or invalid window size");
    return;
  }
  if (!InitReaderIfNeed()) {
    SetError(-1, "init reader failed");
    return;
  }
  if (!InitDecoder()) {
    SetError(-1, "init decoder failed");
    return;
  }
  SetState(PlayerState::kPrepared);
  if (config_.callback) {
    config_.callback->OnPrepared();
  }
  LOGI_TAG(kLogTag) << "init success " << " url: " << config_.url
                    << " disable audio: " << config_.disable_audio
                    << " disable video: " << config_.disable_video;
}

void PlayerImpl::PlayInternal() {
  LOGI_TAG(kLogTag) << "play";
  auto state = GetState();
  if (state == PlayerState::kPrepared || state == PlayerState::kPaused) {
    decoder_task_queue_->PostTask([this]() {
      if (this->audio_render_) {
        this->audio_render_->Play();
      }
      if (this->component_.video_render) {
        this->component_.video_render->Play();
      }
      timer_.Resume();
    });
  } else if (state == PlayerState::kCompleted) {
    SeekInternal(0);
  } else {
    return;
  }
  SetState(PlayerState::kPlaying);
  return;
}

void PlayerImpl::PauseInternal() {
  LOGI_TAG(kLogTag) << "pause";
  SetState(PlayerState::kPaused);
  decoder_task_queue_->PostTask([this]() {
    timer_.Pause();
    if (audio_render_) {
      audio_render_->Pause();
    }
    if (component_.video_render) {
      component_.video_render->Pause();
    }
  });
  return;
}

void PlayerImpl::SeekInternal(int64_t target_ms) {
  if (target_ms < 0) {
    target_ms = 0;
  }
  ++seek_id_;
  start_time_ = target_ms;
  LOGI_TAG(kLogTag) << "seek id: " << seek_id_ << " target ms: " << target_ms;
  decoder_task_queue_->BlockingCall([this, target_ms] {
    if (audio_render_) {
      audio_render_->Flush();
    }
    if (audio_decode_to_render_) {
      audio_decode_to_render_->SetSerial(seek_id_);
      audio_decode_to_render_->SetStartTime(target_ms);
    }
    if (video_decode_to_render_) {
      video_decode_to_render_->SetSerial(seek_id_);
      video_decode_to_render_->SetStartTime(target_ms);
    }
    timer_.SeekTo(target_ms);
    av_packet_queue_.Clear();
  });
  uint32_t seek_id = seek_id_;
  reader_task_queue_->PostTask([this, target_ms, seek_id] {
    if (seek_id != this->seek_id_) {
      LOGI_TAG(kLogTag) << "seek id: " << seek_id << " is not equal to "
                        << this->seek_id_;
      return;
    }
    reader_info_.reader_->SeekTo(target_ms);
    this->reader_info_.serial = seek_id;
  });
}

void PlayerImpl::UnInitInternal() {
  RTC_DCHECK(GetState() != PlayerState::kIdle);
  SetState(PlayerState::kEnded);
  UnInitDecoder();
  UnInitReader();
  return;
}

void PlayerImpl::ReadPacket() {
  bool had_run = false;
  if (ThreadShouldExit()) {
    LOGI_TAG(kLogTag) << "read thread exit id: " << id_;
    return;
  }
  int64_t audio_duration_ms = 0, video_duration_ms = 0;
  if (reader_info_.reader_->HasAudio() && !config_.disable_audio) {
    audio_duration_ms = av_rescale_q(
        av_packet_queue_.GetDuration(false),
        reader_info_.reader_->GetAudioStream()->time_base, {1, 1000});
  }
  if (reader_info_.reader_->HasVideo() && !config_.disable_video) {
    video_duration_ms = av_rescale_q(
        av_packet_queue_.GetDuration(true),
        reader_info_.reader_->GetVideoStream()->time_base, {1, 1000});
  }
  if (audio_duration_ms < max_packet_queue_size_ms_ &&
      video_duration_ms < max_packet_queue_size_ms_ &&
      seek_id_ == reader_info_.serial) {
    reader_info_.packet_.SetSerial(reader_info_.serial);
    auto res =
        reader_info_.reader_->ReadPacket(reader_info_.packet_.GetPacket());
    if (res == core::FfmpegStatus::kOk) {
      auto packet = reader_info_.packet_.GetPacket();
      if (packet->stream_index == reader_info_.reader_->GetAudioStreamIndex() &&
          !config_.disable_audio) {
        reader_info_.packet_.SetTimeBase(
            reader_info_.reader_->GetAudioStream()->time_base);
        av_packet_queue_.PushPacket(reader_info_.packet_, false);
      } else if (packet->stream_index ==
                     reader_info_.reader_->GetVideoStreamIndex() &&
                 !config_.disable_video) {
        reader_info_.packet_.SetTimeBase(
            reader_info_.reader_->GetVideoStream()->time_base);
        av_packet_queue_.PushPacket(reader_info_.packet_, true);
      }
      had_run = true;
    } else if (res == core::FfmpegStatus::kEof) {
      av_packet_queue_.SetEof();
    } else if (res == core::FfmpegStatus::kError) {
      SetError(-1, "read packet error");
    }
  }
  if (had_run) {
    reader_task_queue_->PostTask([this] { this->ReadPacket(); });
  } else {
    reader_task_queue_->PostDelayedTask([this] { this->ReadPacket(); },
                                        webrtc::TimeDelta::Millis(10));
  }
}

bool PlayerImpl::CheckState(PlayerState state) {
  switch (state) {
    case PlayerState::kIdle:
      return player_info_.state == PlayerState::kIdle;
    case PlayerState::kPrepared: {
      return player_info_.state == PlayerState::kIdle;
    }
    case PlayerState::kPlaying:
      return player_info_.state == PlayerState::kPrepared ||
             player_info_.state == PlayerState::kPaused;
    case PlayerState::kPaused:
      return player_info_.state == PlayerState::kPlaying;
    case PlayerState::kCompleted:
      return player_info_.state != PlayerState::kError &&
             player_info_.state != PlayerState::kIdle;
    case PlayerState::kEnded:
    case PlayerState::kError:
      return true;
    default:
      return false;
  }
}

bool PlayerImpl::SetState(PlayerState state) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!CheckState(state)) {
    return false;
  }
  player_info_.state = state;
  return true;
}

PlayerState PlayerImpl::GetState() {
  std::lock_guard<std::mutex> lock(mutex_);
  return player_info_.state;
}

bool PlayerImpl::ThreadShouldExit() {
  auto state = GetState();
  return state == PlayerState::kError || state == PlayerState::kEnded;
}

int PlayerImpl::InitReaderIfNeed() {
  if (reader_info_.reader_is_init) {
    return true;
  }
  reader_task_queue_ = rtc::Thread::Create();
  reader_task_queue_->SetName("reader" + std::to_string(id_), nullptr);
  reader_task_queue_->Start();
  reader_task_queue_->BlockingCall([this] {
    core::ReaderFfmpeg::Config config;
    config.url = config_.url;
    config.disable_audio = config_.disable_audio;
    config.disable_video = config_.disable_video;
    if (!reader_info_.reader_) {
      reader_info_.reader_ = std::make_unique<core::ReaderFfmpeg>();
    }
    reader_info_.serial = seek_id_;
    int ret = reader_info_.reader_->InitReader(config);
    if (ret != 0) {
      SetError(-1, "failed init reader");
    }
    auto audio_stream = reader_info_.reader_->GetAudioStream();
    auto video_stream = reader_info_.reader_->GetVideoStream();
    if (audio_stream == nullptr && video_stream == nullptr) {
      SetError(-1, "can not find audio_stream or video_stream");
    }
    std::stringstream media_info;
    if (audio_stream) {
      media_info << "audio info: "
                 << reader_info_.reader_->GetAudioFormatInfo().ToString()
                 << " duration_ms: "
                 << reader_info_.reader_->GetAudioDurationMs() << "\n ";
    }
    if (video_stream) {
      media_info << " video info: "
                 << reader_info_.reader_->GetVideoFormatInfo().ToString()
                 << " duration_ms: "
                 << reader_info_.reader_->GetVideoDurationMs();
    }
    LOGI_TAG(kLogTag) << "media info: " << media_info.str();
    ReadPacket();
  });
  reader_info_.reader_is_init = true;
  return true;
}

void PlayerImpl::UnInitReader() {
  if (!reader_info_.reader_is_init) {
    return;
  }
  LOGI_TAG(kLogTag) << "UnInitReader";
  reader_task_queue_->BlockingCall(
      [this] { reader_info_.reader_->UnInitReader(); });
  reader_task_queue_->Stop();
  reader_task_queue_->Quit();
  reader_task_queue_.reset();
  reader_info_.reader_.reset();
  reader_info_.reader_is_init = false;
}

bool PlayerImpl::InitDecoder() {
  decoder_task_queue_ = rtc::Thread::Create();
  decoder_task_queue_->SetName("decoder" + std::to_string(id_), nullptr);
  decoder_task_queue_->Start();
  decoder_task_queue_->BlockingCall([this] {
    if (!reader_info_.reader_) {
      SetError(-1, "reader is null");
      return;
    }
    auto audio_stream = reader_info_.reader_->GetAudioStream();
    if (audio_stream) {
      audio_render_ =
          AudioRender::Create(reader_info_.reader_->GetAudioFormatInfo());
      audio_render_->Init();
      audio_render_->Play();
    }
    auto video_stream = reader_info_.reader_->GetVideoStream();
    if (video_stream) {
      if (component_.video_render == nullptr) {
        RTC_CHECK(video_render_factory_);
        component_.video_render = video_render_factory_->Create();
      }
      component_.video_render->Init(config_.window);
      component_.video_render->Play();
    }
    if (!config_.disable_audio && reader_info_.reader_->HasAudio()) {
      audio_decode_to_render_ = std::make_unique<AudioDecodeToRender>();
      AudioDecodeToRender::Config audio_config;
      audio_config.stream = reader_info_.reader_->GetAudioStream();
      audio_config.task_queue = decoder_task_queue_.get();
      audio_config.get_packet = [this](PacketProxy& packet) {
        return GetAudioPacket(packet);
      };
      audio_config.on_error = [this] { OnAudioError(); };
      audio_config.on_completed = [this] { OnAudioCompleted(); };
      audio_config.timer = &timer_;
      audio_config.audio_render = audio_render_.get();
      audio_decode_to_render_->Init(audio_config);
      audio_decode_to_render_->Process();
    }

    if (!config_.disable_video && reader_info_.reader_->HasVideo()) {
      video_decode_to_render_ = std::make_unique<VideoDecodeToRender>();
      VideoDecodeToRender::Config video_config;
      video_config.stream = reader_info_.reader_->GetVideoStream();
      video_config.task_queue = decoder_task_queue_.get();
      video_config.get_packet = [this](PacketProxy& packet) {
        return GetVideoPacket(packet);
      };
      video_config.on_error = [this] { OnVideoError(); };
      video_config.on_completed = [this] { OnVideoCompleted(); };
      video_config.timer = &timer_;
      video_config.video_render = component_.video_render.get();
      video_decode_to_render_->Init(video_config);
      video_decode_to_render_->Process();
    }
  });
  return true;
}

void PlayerImpl::UnInitDecoder() {
  LOGI_TAG(kLogTag) << "UnInitDecoder";
  decoder_task_queue_->BlockingCall([this] {
    if (audio_decode_to_render_) {
      audio_decode_to_render_->UnInit();
    }
    if (video_decode_to_render_) {
      video_decode_to_render_->UnInit();
    }
    if (audio_render_) {
      audio_render_->UnInit();
    }
    if (component_.video_render) {
      component_.video_render->UnInit();
      component_.video_render = nullptr;
    }
  });
  decoder_task_queue_->Stop();
  decoder_task_queue_->Quit();
  if (audio_decode_to_render_) {
    audio_decode_to_render_.reset();
  }
  if (video_decode_to_render_) {
    video_decode_to_render_.reset();
  }
  if (audio_render_) {
    audio_render_.reset();
  }
  if (component_.video_render) {
    component_.video_render = nullptr;
  }
  decoder_task_queue_.reset();
}

void PlayerImpl::SetError(int32_t error_code, const std::string& error_msg) {
  std::lock_guard<std::mutex> lock(mutex_);
  player_info_.state = PlayerState::kError;
  player_task_queue_->PostTask([this, error_code, error_msg] {
    if (config_.callback) {
      config_.callback->OnError(error_code, error_msg);
    }
  });
}

std::unique_ptr<Player> Player::Create(
    util::PointDelegate<webrtc::TaskQueueFactory> task_queue_factory,
    std::unique_ptr<core::VideoRenderFactory> video_render_factory) {
  return std::make_unique<PlayerImpl>(std::move(video_render_factory),
                                      std::move(task_queue_factory));
}

}  // namespace media_demo
