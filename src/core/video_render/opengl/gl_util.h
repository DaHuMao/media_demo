#ifndef CORE_VIDEO_RENDER_OPENGL_GL_UTIL_H_
#define CORE_VIDEO_RENDER_OPENGL_GL_UTIL_H_
#include <cstddef>

#include "rtc_base/checks.h"
#include "util/log.h"
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define ARRAY_SIZE(array) (sizeof(ArraySizeHelper(array)))
bool GlCheckError();
bool GlHasError();
void Print(float *ptr, int raw, int col);

#define PRINT_MODEL_COUNT(num, model) \
  static int count = num;             \
  if (count-- > 0) {                  \
    Print(model.Ptr(), 4, 4);         \
  }

#define RUN_FUNC_CHECK(gl_func)                                       \
  gl_func;                                                            \
  if (GlCheckError()) {                                               \
    RTC_CHECK(false) << __FILE__ << " line: " << __LINE__             \
                     << " function: " << __FUNCTION__ << ": error";   \
    LOGI_TAG("OpenGL") << __FILE__ << " line: " << __LINE__           \
                       << " function: " << __FUNCTION__ << ": error"; \
  }
#endif  // CORE_VIDEO_RENDER_OPENGL_GL_UTIL_H_
