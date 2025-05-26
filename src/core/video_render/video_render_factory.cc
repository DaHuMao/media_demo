#include "core/video_render/video_render_factory.h"

#include "core/video_render/video_render.h"
#if defined(MEDIA_ANDROID)
#include "core/video_render/video_render_jni.h"
#else
#include "core/video_render/video_render_impl.h"
#endif
namespace core {
class VideoRenderFactoryImpl : public VideoRenderFactory {
 public:
  std::unique_ptr<VideoRender> Create() override {
#if defined(MEDIA_MAC)
    return std::make_unique<VideoRenderImpl>();
#elif  defined(MEDIA_ANDROID)
    return nullptr;
#else
    return nullptr;
#endif
  }
};

std::unique_ptr<VideoRenderFactory> VideoRenderFactory::CreateDefaultFactory() {
  return std::make_unique<VideoRenderFactoryImpl>();
}

}  // namespace core
