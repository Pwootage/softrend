#include "ColorTFramebuffer.hpp"

softrend::ColorTFramebuffer::ColorTFramebuffer(softrend::fb_pos_t width, softrend::fb_pos_t height) {
  this->width = width;
  this->height = height;
  colorBuffer = static_cast<color_t *>(malloc(sizeof(color_t) * width * height));
  depthBuffer = static_cast<float *>(malloc(sizeof(float) * width * height));
}

void softrend::ColorTFramebuffer::putPixel(softrend::fb_pos_t pos, softrend::color_t color, float depth) {

  if (depthBuffer[pos] <= depth) return;
  depthBuffer[pos] = depth;

//  this->framebuffer[idx].align = rgba.align;
//  this->framebuffer[idx] = rgba;
// we take a block here depending on what's defined
#ifdef __SSE2__
#ifdef F32_COLOR
  *reinterpret_cast<__m128i *>(&this->colorBuffer[pos]) = *reinterpret_cast<const __m128i *>(&color);
#else
  *reinterpret_cast<uint32_t *>(&this->colorBuffer[pos]) = *reinterpret_cast<const uint32_t *>(&color);
#endif
#elif __ARM_NEON
  *reinterpret_cast<float32x4_t *>(&this->colorBuffer[pos]) = *reinterpret_cast<const float32x4_t *>(&color);
#else
    this->colorBuffer[pos] = color;
#endif
}

void softrend::ColorTFramebuffer::clear(const color_t &clearColor, bool colorBuffer, bool depthBuffer) {
    float clearDepth = std::numeric_limits<float>::quiet_NaN();
    for (fb_pos_t i = 0; i < width * height; i++) {
      if (colorBuffer) this->colorBuffer[i] = clearColor;
      if (depthBuffer) this->depthBuffer[i] = clearDepth;
    }
}

softrend::color_t softrend::ColorTFramebuffer::getPixel(const fb_pos_t &pos) const {
  return colorBuffer[pos];
}

float softrend::ColorTFramebuffer::getDepth(fb_pos_t pos) const {
  return depthBuffer[pos];
}

softrend::fb_pos_t softrend::ColorTFramebuffer::getWidth() const {
  return width;
}

softrend::fb_pos_t softrend::ColorTFramebuffer::getHeight() const {
  return height;
}
softrend::ColorTFramebuffer::~ColorTFramebuffer() {
  free(colorBuffer);
  free(depthBuffer);
}

const softrend::color_t *softrend::ColorTFramebuffer::getRawColorBuffer() const {
  return colorBuffer;
}

const float *softrend::ColorTFramebuffer::getRawDepthBuffer() const {
  return depthBuffer;
}

