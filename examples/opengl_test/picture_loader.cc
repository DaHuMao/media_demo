#include "examples/opengl_test/picture_loader.h"
#include "examples/opengl_test/stb_image/stb_image.h"

PictureLoader::PictureLoader(const std::string& file) {
  int channels;
  stbi_set_flip_vertically_on_load(true);
  data_ = stbi_load(file.c_str(), &size_.width, &size_.height, &channels, 0);
  if (channels == 4) {
    data_tmp_ = new uint8_t[size_.width * size_.height * 3];
    for (int i = 0; i < size_.width * size_.height; ++i) {
      data_tmp_[i * 3] = data_[i * 4];
      data_tmp_[i * 3 + 1] = data_[i * 4 + 1];
      data_tmp_[i * 3 + 2] = data_[i * 4 + 2];
    }
  }
}

PictureLoader::~PictureLoader() {
  if (data_) {
    stbi_image_free(data_);
  }
  if (data_tmp_) {
    delete[] data_tmp_;
  }
}
uint8_t* PictureLoader::GetData() const {
  return data_tmp_ ? data_tmp_ : data_;
}

core::VideoSize PictureLoader::GetSize() const { return size_; }
