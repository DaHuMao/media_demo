#ifndef PLAYER_PLAYER_DEFINE_H_
#define PLAYER_PLAYER_DEFINE_H_
namespace media_demo {
enum class PlayerState {
  kIdle,
  kPrepared,
  kPlaying,
  kPaused,
  kCompleted,
  kEnded,
  kError,
};
} // media_demo
#endif // PLAYER_PLAYER_DEFINE_H_
