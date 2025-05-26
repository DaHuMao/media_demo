#ifndef CORE_VIDEO_OPENGL_CONST_DATA_H_
#define CORE_VIDEO_OPENGL_CONST_DATA_H_
#include <cstddef>
#include <cstdint>

#include "core/video_render/opengl/mat_util.h"
constexpr size_t kPointCount = 8;
constexpr size_t kOneDimSize = 8;
constexpr core::MatUtil::Coordinate3D kCubeCoordinate[kPointCount] = {
    {0.5f, 0.5f, 0.5f},    {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f},   {0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}};
constexpr float kCubePositions[kPointCount * kOneDimSize] = {
    //     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -  顶点法线
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 右上
    0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // 右下
    -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 左下
    -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // 左上
    0.5f,  0.5f,  -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 右上
    0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // 右下
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // 左下
    -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f   // 左上
};
constexpr uint32_t kCubeIndices[] = {
    // front
    0, 1, 2, 2, 3, 0,

    // right
    4, 5, 1, 1, 0, 4,

    // back
    4, 5, 6, 6, 7, 4,

    // left
    6, 7, 3, 3, 2, 6,

    // obove
    0, 4, 7, 7, 3, 0,

    // under
    1, 5, 6, 6, 2, 1};
#endif  // CORE_VIDEO_OPENGL_CONST_DATA_H_
