#ifndef SOFTREND_RGBA128FRAMEBUFFER_HPP
#define SOFTREND_RGBA128FRAMEBUFFER_HPP

#include "Framebuffer.hpp"

namespace softrend {

class ColorTFramebuffer : public Framebuffer {
public:
  ColorTFramebuffer(fb_pos_t width, fb_pos_t height);
  virtual ~ColorTFramebuffer();

  void putPixel(fb_pos_t pos, color_t color, float depth) override;

  void clear(const color_t &clearColor, bool colorBuffer, bool depthBuffer) override;

  color_t getPixel(const fb_pos_t &pos) const override;

  float getDepth(fb_pos_t pos) const override;

  fb_pos_t getWidth() const override;

  fb_pos_t getHeight() const override;

  const color_t *getRawColorBuffer() const;
  const float *getRawDepthBuffer() const;

private:
  fb_pos_t width;
  fb_pos_t height;
  color_t *colorBuffer;
  float *depthBuffer;
};


}


#endif //SOFTREND_RGBA128FRAMEBUFFER_HPP
