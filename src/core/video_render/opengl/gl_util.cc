#include <GL/glew.h>

#include <iostream>

#include "util/log.h"
static bool sg_gl_has_error = false;
const char kLogTag[] = "GlUtil";
bool GlCheckError() {
  bool res = false;
  while (GLenum error = glGetError()) {
    res = true;
    sg_gl_has_error = true;
    LOGE_TAG(kLogTag) << "[OpenGl Error]" << std::hex << error
      << std::dec;
  }
  return res;
}

bool GlHasError() { return sg_gl_has_error; }

void Print(float* ptr, int raw, int col) {
  std::cout << "ptr: " << ptr << std::endl;
  for (int i = 0; i < raw; ++i) {
    for (int j = 0; j < col; ++j)
      std::cout << ptr[i * col + j] << " ";
    std::cout << std::endl;
  }
}
