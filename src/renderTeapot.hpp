#ifndef SOFTREND_RENDERTEAPOT_HPP
#define SOFTREND_RENDERTEAPOT_HPP

#include "SoftwareRasterizer.hpp"

namespace renderTeapot {

void init(const char* modelPath);
void render(size_t frame);
const softrend::color_t *getFB();
size_t getFBWidth();
size_t getFBHeight();

};

#endif //SOFTREND_RENDERTEAPOT_HPP
