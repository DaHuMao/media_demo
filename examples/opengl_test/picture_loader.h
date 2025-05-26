#ifndef OPENGL_TEST_PICTURE_LOADER_H_
#define OPENGL_TEST_PICTURE_LOADER_H_
#include <string>

#include "core/video_common/video_format_define.h"
class PictureLoader {
 public:
  PictureLoader(const std::string& file);
  ~PictureLoader();
  uint8_t* GetData() const;
  core::VideoSize GetSize() const;

 private:
  uint8_t* data_ = nullptr;
  uint8_t* data_tmp_ = nullptr;
  core::VideoSize size_;
};
#endif  // OPENGL_TEST_PICTURE_LOADER_H_
