#ifndef SOFTREND_RGBA128FRAMEBUFFER_HPP
#define SOFTREND_RGBA128FRAMEBUFFER_HPP

#include "Framebuffer.hpp"

namespace softrend {

template<
    typename color_t, typename array_t,
    typename toArrayT, typename toColorT
>
class ArrayFramebuffer : public Framebuffer<color_t> {
  static_assert((sizeof(color_t) & (sizeof(color_t) - 1)) == 0, "color_t must be a size that is a power of two");
public:
  ArrayFramebuffer(fb_pos_t width, fb_pos_t height) :
      width(width), height(height) {
    // colorBuffer = new array_t[width * height];
#ifdef __APPLE__
    void *ptr = nullptr;
    posix_memalign(&ptr, sizeof(array_t), sizeof(array_t) * width * height);
    colorBuffer = static_cast<array_t*>(ptr);
#else
    colorBuffer = static_cast<array_t *>(std::aligned_alloc(sizeof(array_t), sizeof(array_t) * width * height));
#endif
    printf("align: %lx, %lx\n", alignof(color_t), reinterpret_cast<uint64_t>(colorBuffer) & 0xFFFFu);
    depthBuffer = static_cast<float *>(malloc(sizeof(float) * width * height));
  }

  virtual ~ArrayFramebuffer() {
    // free(colorBuffer);
    delete[] colorBuffer;
    free(depthBuffer);
  }

  void putPixel(fb_pos_t pos, color_t color, float depth) override {
    if (depthBuffer[pos] <= depth) return;
    depthBuffer[pos] = depth;
    this->colorBuffer[pos] = convToArray(color);

//  this->framebuffer[idx].align = rgba.align;
//  this->framebuffer[idx] = rgba;
// we take a block here depending on what's defined
// #ifdef __SSE2__
// #ifdef F32_COLOR
//   *reinterpret_cast<__int128 *>(&this->colorBuffer[pos]) = *reinterpret_cast<const __int128 *>(&color);
// #else
//   *reinterpret_cast<uint32_t *>(&this->colorBuffer[pos]) = *reinterpret_cast<const uint32_t *>(&color);
// #endif
// #elif __ARM_NEON
//   *reinterpret_cast<float32x4_t *>(&this->colorBuffer[pos]) = *reinterpret_cast<const float32x4_t *>(&color);
// #else
//     this->colorBuffer[pos] = color;
// #endif
  }

  void clear(const color_t &clearColor, bool clearColorBuffer, bool clearDepthBuffer) override {
    float clearDepth = std::numeric_limits<float>::quiet_NaN();
    for (fb_pos_t i = 0; i < width * height; i++) {
      if (clearColorBuffer) this->colorBuffer[i] = clearColor;
      if (clearDepthBuffer) this->depthBuffer[i] = clearDepth;
    }
  }

  [[nodiscard]] const color_t &getPixel(const fb_pos_t &pos) const override {
    return convToColor(colorBuffer[pos]);
  }

  [[nodiscard]] float getDepth(fb_pos_t pos) const override {
    return depthBuffer[pos];
  }

  [[nodiscard]] fb_pos_t getWidth() const override {
    return width;
  }

  [[nodiscard]] fb_pos_t getHeight() const override {
    return height;
  }

  [[nodiscard]] const color_t *getRawColorBuffer() const {
    return colorBuffer;
  }

  [[nodiscard]] const float *getRawDepthBuffer() const {
    return depthBuffer;
  }

private:
  const fb_pos_t width;
  const fb_pos_t height;
  array_t *colorBuffer;
  depthbuffer_t *depthBuffer;
  toArrayT convToArray;
  toColorT convToColor;
};

using F32ColorFramebuffer = ArrayFramebuffer<
    f32_color_t,
    f32_color_t,
    color_conversion::no_convert<f32_color_t>,
    color_conversion::no_convert<f32_color_t>
>;

}

#endif //SOFTREND_RGBA128FRAMEBUFFER_HPP
