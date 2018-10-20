#include <cstdlib>
#include <cmath>
#include <iostream>

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_integer.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SoftwareRasterizer.hpp"

using namespace std;
using namespace glm;

SoftwareRasterizer::SoftwareRasterizer(int width, int height) : width(width), height(height), widthMax(width - 1),
                                                                heightMax(height - 1) {
  this->framebuffer = new framebuffer_t[width * height];
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

inline void SoftwareRasterizer::drawPixel(const ivec2 &pos, const color_t &rgba) {
//  assert(pos.x >= 0);
//  assert(pos.y >= 0);
//  if (pos.x < 0 || pos.y < 0) {
//    cout << "UHH" << endl;
//  }
  this->framebuffer[pos.x + pos.y * width] = rgba;
}

inline void SoftwareRasterizer::drawPixel(fb_pos_t idx, const color_t &rgba) {
  this->framebuffer[idx] = rgba;
}

void SoftwareRasterizer::clear() {
  for (fb_pos_t y = 0; y < this->getWidth(); y++) {
    fb_pos_t yOff = y * width;
    for (fb_pos_t x = 0; x < this->getWidth(); x++) {
      this->drawPixel(x + yOff, this->currentColor);
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
      this->drawPixel(x + yOff, this->getCurrentColor());
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
      this->drawPixel(x + y * width, this->getCurrentColor());
    }
    if (d > 0) {
      x = x + xi;
      d = d - 2 * dy;
    }
    d = d + 2 * dx;
  }
}

void SoftwareRasterizer::drawLine(const glm::ivec2 &a, const glm::ivec2 &b) {
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
      this->drawPixel(x0 + y * width, this->currentColor);
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
      this->drawPixel(x + yOff, this->currentColor);
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

void SoftwareRasterizer::drawTriLines(const glm::ivec2 &a, const glm::ivec2 &b, const glm::ivec2 &c) {
  this->drawLine(a, b);
  this->drawLine(b, c);
  this->drawLine(c, a);
}

inline float vecSign(ivec2 a, ivec2 b, ivec2 c) {
  return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
}

void SoftwareRasterizer::drawTriFilled(glm::ivec2 a, glm::ivec2 b, glm::ivec2 c) {
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
      fb_pos_t startX = glm::clamp((fb_pos_t)x0, 0, widthMax);
      fb_pos_t endX = glm::clamp((fb_pos_t)x1, 0, widthMax);
      for (fb_pos_t x = startX; x < endX; x++) {
        drawPixel(x + yOff, currentColor);
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
      fb_pos_t startX = glm::clamp((fb_pos_t)x0, 0, widthMax);
      fb_pos_t endX = glm::clamp((fb_pos_t)x1, 0, widthMax);
      for (fb_pos_t x = startX; x < endX; x++) {
        drawPixel(x + yOff, currentColor);
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
//        drawPixel(x + yOff, currentColor);
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
//        drawPixel(x + yOff, this->currentColor);
//      }
//    }
//  }

  // sort A/B/C by y (we'll draw top then bottom half)
//  if (a.y > b.y) {
//    drawTriFilled(b, a, c);
//  } else if (b.y > c.y) {
//    drawTriFilled(a, c, b);
//  } else {
//    for (fb_pos_t y = a.y; y <= c.y; y++) {
//      this->fillTriTop(a, b, c);
//    }
//  }
}
