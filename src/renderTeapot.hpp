#ifndef SOFTREND_RENDERTEAPOT_HPP
#define SOFTREND_RENDERTEAPOT_HPP

#include "SoftwareRasterizer.hpp"

namespace renderTeapot {

struct InitData {
  const char* modelPath = nullptr;
  const char* modelSrc = nullptr;
  size_t modelLen = 0;
  size_t fb_width = 0;
  size_t fb_height = 0;
};

void init(const InitData &initData);
void render(size_t frame);
const softrend::color_t *getFB();
size_t getFBWidth();
size_t getFBHeight();

};

#endif //SOFTREND_RENDERTEAPOT_HPP