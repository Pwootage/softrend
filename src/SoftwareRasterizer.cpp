#include <cstdlib>
#include <cmath>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_integer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/simd/common.h>

#include "SoftwareRasterizer.hpp"

using namespace std;
using namespace glm;

SoftwareRasterizer::SoftwareRasterizer(int width, int height) : width(width), height(height), widthMax(width - 1),
                                                                heightMax(height - 1) {
  cout << sizeof(color_t::data);

  this->framebuffer = static_cast<framebuffer_t *>(malloc(sizeof(framebuffer_t) * width * height));
}

int SoftwareRasterizer::getWidth() const {
  return width;
}

int SoftwareRasterizer::getHeight() const {
  return height;
}

const framebuffer_t *SoftwareRasterizer::getFramebuffer() const {
  return framebuffer;
}

inline void SoftwareRasterizer::drawScreenSpacePixel(fb_pos_t idx, const color_t &rgba) {
//  this->framebuffer[idx].align = rgba.align;
//  this->framebuffer[idx] = rgba;
// we take a block here depending on what's defined
#ifdef F32_COLOR
  *reinterpret_cast<__m128i *>(&this->framebuffer[idx]) = *reinterpret_cast<const __m128i *>(&rgba);
#else
  *reinterpret_cast<uint32_t *>(&this->framebuffer[idx]) = *reinterpret_cast<const uint32_t *>(&rgba);
#endif
//  if (sizeof(color_t::data) == 4) {
//    *reinterpret_cast<uint32_t *>(&this->framebuffer[idx]) = *reinterpret_cast<const uint32_t *>(&rgba);
//  } else if (sizeof(color_t::data) == 16) {
//    *reinterpret_cast<__m128i *>(&this->framebuffer[idx]) = *reinterpret_cast<const __m128i *>(&rgba);
//  } else {
//    this->framebuffer[idx] = rgba;
//  }
//  *reinterpret_cast<uint32_t *>(&this->framebuffer[idx]) = *reinterpret_cast<const uint32_t *>(&rgba);
//  memcpy(&this->framebuffer[idx], &rgba, sizeof(color_t));
//  this->framebuffer[idx] = rgba;

}

inline void SoftwareRasterizer::drawScreenSpacePixel(const ivec2 &pos, const color_t &rgba) {
  drawScreenSpacePixel(pos.x + pos.y * width, rgba);
}

#define DRAW_PIXEL(idx, color) ()

void SoftwareRasterizer::clear() {
  for (fb_pos_t y = 0; y < height; y++) {
    fb_pos_t yOff = y * width;
    for (fb_pos_t x = 0; x < width; x++) {
      this->drawScreenSpacePixel(x + yOff, this->currentColor);
    }
  }
}

inline void SoftwareRasterizer::drawLineLow(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1) {
  fb_pos_t dx = x1 - x0;
  fb_pos_t dy = y1 - y0;
  fb_pos_t yi = 1;
  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }
  fb_pos_t d = 2 * dy - dx;
  fb_pos_t y = y0;
  fb_pos_t yOff = y * width;

  for (fb_pos_t x = x0; x <= x1; x++) {
    if (y > 0 && y < this->getHeight()
        && x > 0 && x < this->getWidth()) {
      this->drawScreenSpacePixel(x + yOff, this->getCurrentColor());
    }
    if (d > 0) {
      y = y + yi;
      yOff = y * width;
      d = d - 2 * dx;
    }
    d = d + 2 * dy;
  }
}

inline void SoftwareRasterizer::drawLineHigh(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1) {
  fb_pos_t dx = x1 - x0;
  fb_pos_t dy = y1 - y0;
  fb_pos_t xi = 1;
  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }
  fb_pos_t d = 2 * dx - dy;
  fb_pos_t x = x0;

  for (fb_pos_t y = y0; y <= y1; y++) {
    if (y > 0 && y < height
        && x > 0 && x < width) {
      this->drawScreenSpacePixel(x + y * width, this->getCurrentColor());
    }
    if (d > 0) {
      x = x + xi;
      d = d - 2 * dy;
    }
    d = d + 2 * dx;
  }
}

void SoftwareRasterizer::drawScreenSpaceLine(const glm::ivec2 &a, const glm::ivec2 &b) {
  fb_pos_t
    x0 = a.x, y0 = a.y,
    x1 = b.x, y1 = b.y;

  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  if (x0 == x1) {
    if (x0 < 0 || x0 >= this->width) {
      return;
    }
    if (y0 > y1) {
      swap(y0, y1);
    }
    y0 = std::max(0, y0);
    y1 = std::min(this->heightMax, y1);
    for (fb_pos_t y = y0; y <= y1; y++) {
      this->drawScreenSpacePixel(x0 + y * width, this->currentColor);
    }
  } else if (y0 == y1) {
    if (y0 < 0 || y0 >= this->height) {
      return;
    }
    if (x0 > x1) {
      swap(x0, x1);
    }
    x0 = std::max(0, x0);
    x1 = std::min(this->widthMax, x1);
    fb_pos_t yOff = y0 * width;
    for (fb_pos_t x = x0; x <= x1; x++) {
      this->drawScreenSpacePixel(x + yOff, this->currentColor);
    }
  } else if (abs(x0 - x1) < abs(y0 - y1)) {
    if (y0 > y1) {
      drawLineHigh(x1, y1, x0, y0);
    } else {
      drawLineHigh(x0, y0, x1, y1);
    }
  } else {
    if (x0 > x1) {
      drawLineLow(x1, y1, x0, y0);
    } else {
      drawLineLow(x0, y0, x1, y1);
    }
  }
}

color_t SoftwareRasterizer::getCurrentColor() const {
  return currentColor;
}

void SoftwareRasterizer::setCurrentColor(color_t currentColor) {
  SoftwareRasterizer::currentColor = currentColor;
}

void SoftwareRasterizer::drawScreenSpaceTriLines(const glm::ivec2 &a, const glm::ivec2 &b, const glm::ivec2 &c) {
  this->drawScreenSpaceLine(a, b);
  this->drawScreenSpaceLine(b, c);
  this->drawScreenSpaceLine(c, a);
}

void SoftwareRasterizer::drawScreenSpaceTriFilled(glm::ivec2 a, glm::ivec2 b, glm::ivec2 c) {
  // Sort our tris by y-coord
  if (a.y > b.y) swap(a, b);
  if (b.y > c.y) swap(b, c);
  if (a.y > b.y) swap(a, b);

  // ok we're sorted now
  fb_pos_t totalHeight = c.y - a.y;
  if (totalHeight == 0) {
    return; // can't draw nuthin
  }
  // bottom half
  fb_pos_t y0 = glm::clamp(a.y, 0, heightMax),
    y1 = glm::clamp(b.y, 0, heightMax),
    y2 = glm::clamp(c.y, 0, heightMax);
  { // top
    float invSlopeA = (float) (b.x - a.x) / (float) (b.y - a.y);
    float invSlopeB = (float) (c.x - a.x) / (float) (c.y - a.y);
    if (invSlopeA > invSlopeB) swap(invSlopeA, invSlopeB);

    float x0 = a.x;
    float x1 = a.x;
    for (fb_pos_t y = y0; y <= y1; y++) {
      fb_pos_t yOff = y * width;
      fb_pos_t startX = glm::clamp((fb_pos_t) x0, 0, widthMax);
      fb_pos_t endX = glm::clamp((fb_pos_t) x1, 0, widthMax);
      for (fb_pos_t x = startX; x < endX; x++) {
        drawScreenSpacePixel(x + yOff, currentColor);
      }

      x0 += invSlopeA;
      x1 += invSlopeB;
    }
  }

  { // bottom
    float invSlopeA = (float) (c.x - a.x) / (float) (c.y - a.y);
    float invSlopeB = (float) (c.x - b.x) / (float) (c.y - b.y);
    if (invSlopeA < invSlopeB) swap(invSlopeA, invSlopeB);

    float x0 = c.x;
    float x1 = c.x;
    for (fb_pos_t y = y2; y > y1; y--) {
      fb_pos_t yOff = y * width;
      fb_pos_t startX = glm::clamp((fb_pos_t) x0, 0, widthMax);
      fb_pos_t endX = glm::clamp((fb_pos_t) x1, 0, widthMax);
      for (fb_pos_t x = startX; x < endX; x++) {
        drawScreenSpacePixel(x + yOff, currentColor);
      }

      x0 -= invSlopeA;
      x1 -= invSlopeB;
    }
  }
//  { // bottom
//    fb_pos_t topHeight = b.y - a.y + 1;
//    for (fb_pos_t y = y0; y <= y1; y++) {
//      fb_pos_t yOff = y * width;
//      float slopeA = (float) (y - a.y) / totalHeight;
//      float slopeB = (float) (y - a.y) / topHeight;
//      fb_pos_t x0 = a.x + (fb_pos_t) ((c.x - a.x) * slopeA);
//      fb_pos_t x1 = a.x + (fb_pos_t) ((b.x - a.x) * slopeB);
//
//      if (x0 > x1) swap(x0, x1);
//      x0 = glm::clamp(x0, 0, width);
//      x1 = glm::clamp(x1, 0, width);
//
//      for (fb_pos_t x = x0; x < x1; x++) {
//        drawScreenSpacePixel(x + yOff, currentColor);
//      }
//    }
//  }


//  //TODO: don't use bounding box, and fill it properly
//  fb_pos_t
//    x0 = std::min(a.x, std::min(b.x, c.x)),
//    x1 = std::max(a.x, std::max(b.x, c.x)),
//    y0 = std::min(a.y, std::min(b.y, c.y)),
//    y1 = std::max(a.y, std::max(b.y, c.y));
//
//  x0 = std::max(x0, 0);
//  x1 = std::min(x1, width - 1);
//  y0 = std::max(y0, 0);
//  y1 = std::min(y1, height - 1);
//
//  for (fb_pos_t y = y0; y <= y1; y++) {
//    fb_pos_t yOff = y * width;
//    for (fb_pos_t x = x0; x <= x1; x++) {
//      ivec2 p(x, y);
//      bool b1 = vecSign(p, a, b) < 0;
//      bool b2 = vecSign(p, b, c) < 0;
//      bool b3 = vecSign(p, c, a) < 0;
//      if ((b1 == b2) && (b2 == b3)) {
//        drawScreenSpacePixel(x + yOff, this->currentColor);
//      }
//    }
//  }

  // sort A/B/C by y (we'll draw top then bottom half)
//  if (a.y > b.y) {
//    drawScreenSpaceTriFilled(b, a, c);
//  } else if (b.y > c.y) {
//    drawScreenSpaceTriFilled(a, c, b);
//  } else {
//    for (fb_pos_t y = a.y; y <= c.y; y++) {
//      this->fillTriTop(a, b, c);
//    }
//  }
}

void SoftwareRasterizer::drawClipSpaceTriangle(const glm::vec4 &a, const glm::vec4 &b, const glm::vec4 &c) {
  // Alrighty we're in clip space, let's transform to screen space first

  // atm this is also NDC, we'll come back to that
  float halfW = width / 2.f;
  float halfH = height / 2.f;

  // NDC
//  vec3 aa = a / a.w;
//  vec3 bb = b / b.w;
//  vec3 cc = c / c.w;

  // screen-space
  ivec2 ai = {(a.x / a.w + 1.f) * halfW, (a.y / a.w + 1.f) * halfH};
  ivec2 bi = {(b.x / b.w + 1.f) * halfW, (b.y / a.w + 1.f) * halfH};
  ivec2 ci = {(c.x / c.w + 1.f) * halfW, (c.y / a.w + 1.f) * halfH};

//  cout << to_string(a) << "->" << to_string(ai) << endl;
//  cout << to_string(b) << "->" << to_string(bi) << endl;
//  cout << to_string(c) << "->" << to_string(ci) << endl;

  drawScreenSpaceTriFilled(ai, bi, ci);

//  cout << endl;

}

void SoftwareRasterizer::drawClipSpaceIndexed(SoftwareRasterizer::DrawMode mode,
                                              const VertexBuffer<glm::vec4> &verts,
                                              const IndexBuffer &indicies) {
  if (mode == SoftwareRasterizer::DrawMode::TRAINGLES) {
    for (size_t i = 0, end = indicies.size(); i + 2 < end; i += 3) {
      drawClipSpaceTriangle(
        verts[indicies[i]],
        verts[indicies[i + 1]],
        verts[indicies[i + 2]]
      );
    }
  } else if (mode == SoftwareRasterizer::DrawMode::TRIANGLE_STRIP) {
    for (size_t i = 0, end = indicies.size(); i + 2 < end; i++) {

      if (i % 2 == 0) {
        drawClipSpaceTriangle(
          verts[indicies[i]],
          verts[indicies[i + 1]],
          verts[indicies[i + 2]]
        );
      } else {
        drawClipSpaceTriangle(
          verts[indicies[i + 2]],
          verts[indicies[i + 1]],
          verts[indicies[i]]
        );
      }
    }
  } else if (mode == SoftwareRasterizer::DrawMode::TRIANGLE_FAN) {
    for (size_t i = 1, end = indicies.size(); i + 1 < end; i++) {
      drawClipSpaceTriangle(
        verts[indicies[0]],
        verts[indicies[i]],
        verts[indicies[i + 1]]
      );
    }
  }
}

