#ifndef SOFTREND_GBAMODE5FRAMEBUFFER_HPP
#define SOFTREND_GBAMODE5FRAMEBUFFER_HPP

#include "../src/buffers/Framebuffer.hpp"

class GBAMode3Framebuffer : public softrend::Framebuffer {
public:
  GBAMode3Framebuffer();

  void putPixel(softrend::fb_pos_t pos, softrend::color_t color, float depth) override;

  void clear(const softrend::color_t &clearColor, bool colorBuffer, bool depthBuffer) override;

  softrend::color_t getPixel(const softrend::fb_pos_t &pos) const override;

  float getDepth(softrend::fb_pos_t pos) const override;

  softrend::fb_pos_t getWidth() const override;

  softrend::fb_pos_t getHeight() const override;

  float *depthBuffer;
};


#endif //SOFTREND_GBAMODE5FRAMEBUFFER_HPP
