#include <gba_video.h>

#include "GBAMode3Framebuffer.hpp"

using namespace softrend;

GBAMode3Framebuffer::GBAMode3Framebuffer() {
  depthBuffer = static_cast<float *>(malloc(sizeof(float) * getWidth() * getHeight()));
}

void GBAMode3Framebuffer::putPixel(fb_pos_t pos, color_t color, float depth) {
  if (depthBuffer[pos] <= depth) return;
  depthBuffer[pos] = depth;

  u16 r = static_cast<u16>(color.r * 0x1F);
  u16 g = static_cast<u16>(color.g * 0x1F);
  u16 b = static_cast<u16>(color.b * 0x1F);

  r = glm::clamp(r, (u16)0, (u16)0x1F);
  g = glm::clamp(g, (u16)0, (u16)0x1F);
  b = glm::clamp(b, (u16)0, (u16)0x1F);

  u16* MODE3_RAW_FB = ((u16*)0x06000000);
  MODE3_RAW_FB[pos] = RGB5(r,g,b);
}

void GBAMode3Framebuffer::clear(const color_t &clearColor, bool colorBuffer, bool depthBuffer) {
  u16 r = static_cast<u16>(clearColor.r * 0x1F);
  u16 g = static_cast<u16>(clearColor.g * 0x1F);
  u16 b = static_cast<u16>(clearColor.b * 0x1F);

  r = glm::clamp(r, (u16)0, (u16)0x1F);
  g = glm::clamp(g, (u16)0, (u16)0x1F);
  b = glm::clamp(b, (u16)0, (u16)0x1F);
  u16 color = RGB5(r,g,b);

  u16* MODE3_RAW_FB = ((u16*)0x06000000);

  float clearDepth = std::numeric_limits<float>::quiet_NaN();
  for (fb_pos_t i = 0; i < getWidth() * getHeight(); i++) {
    if (colorBuffer) MODE3_RAW_FB[i] = color;
    if (depthBuffer) this->depthBuffer[i] = clearDepth;
  }
}

color_t GBAMode3Framebuffer::getPixel(const fb_pos_t &pos) const {
  return color_t();
}

float GBAMode3Framebuffer::getDepth(fb_pos_t pos) const {
  return 0;
}

fb_pos_t GBAMode3Framebuffer::getWidth() const {
  return 240;
}

fb_pos_t GBAMode3Framebuffer::getHeight() const {
  return 160;
}
