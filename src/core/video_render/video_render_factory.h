#ifndef CORE_VIDEO_RENDER_VIDEO_RENDER_FACTORY_H_
#define CORE_VIDEO_RENDER_VIDEO_RENDER_FACTORY_H_
#ifdef MEDIA_ANDROID
#include <jni.h>
#endif
#include "core/video_render/video_render.h"
namespace core {
class VideoRenderFactory {
 public:
  virtual ~VideoRenderFactory() = default;
  virtual std::unique_ptr<VideoRender> Create() = 0;
  static std::unique_ptr<VideoRenderFactory> CreateDefaultFactory();
};
}  // namespace core
#endif  // CORE_VIDEO_RENDER_VIDEO_RENDER_FACTORY_H_
