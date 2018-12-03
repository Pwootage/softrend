#ifndef SOFTREND_SOFTRENDERER_HPP
#define SOFTREND_SOFTRENDERER_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtx/type_aligned.hpp>
#include "buffers/VertexBuffer.hpp"
#include "buffers/IndexBuffer.hpp"

namespace softrend {

#define F32_COLOR
#ifdef F32_COLOR
using color_t = glm::aligned_f32vec4;
#else
using color_t = glm::aligned_u8vec4;
#endif

using framebuffer_t = color_t;
using fb_pos_t = int32_t;

enum class DrawMode {
  TRAINGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN
};

// hold on to something, we're about to template like nothing you've seen before
template<
  typename VertexType, typename FragmentType
>
class SoftwareRasterizer {
public:

  SoftwareRasterizer(int width, int height)
    : width(width),
      height(height),
      widthMax(width - 1),
      heightMax(height - 1) {

    std::cout << sizeof(color_t::data);

    this->framebuffer = static_cast<framebuffer_t *>(malloc(sizeof(framebuffer_t) * width * height));
  }

  // basic members
  int getWidth() const {
    return width;
  }

  int getHeight() const {
    return height;
  }

  const framebuffer_t *getFramebuffer() const {
    return framebuffer;
  }

  // Methods
  void clear() {
    for (fb_pos_t y = 0; y < height; y++) {
      fb_pos_t yOff = y * width;
      for (fb_pos_t x = 0; x < width; x++) {
        this->drawScreenSpacePixel(x + yOff, this->currentColor);
      }
    }
  }

  // state (2d)
  color_t getCurrentColor() const {
    return currentColor;
  }

  void setCurrentColor(color_t currentColor) {
    SoftwareRasterizer::currentColor = currentColor;
  }

  // ScreenSpace
  inline void drawScreenSpacePixel(fb_pos_t idx, const color_t &rgba) {
//  this->framebuffer[idx].align = rgba.align;
//  this->framebuffer[idx] = rgba;
// we take a block here depending on what's defined
#ifdef F32_COLOR
    *reinterpret_cast<__m128i *>(&this->framebuffer[idx]) = *reinterpret_cast<const __m128i *>(&rgba);
#else
    *reinterpret_cast<uint32_t *>(&this->framebuffer[idx]) = *reinterpret_cast<const uint32_t *>(&rgba);
#endif
  }

  inline void drawScreenSpacePixel(const glm::ivec2 &pos, const color_t &rgba) {
    drawScreenSpacePixel(pos.x + pos.y * width, rgba);
  }

  void drawScreenSpaceLine(const FragmentType &a, const FragmentType &b) {
    fb_pos_t
      x0 = a.x, y0 = a.y,
      x1 = b.x, y1 = b.y;

    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    if (x0 == x1) {
      if (x0 < 0 || x0 >= this->width) {
        return;
      }
      if (y0 > y1) {
        std::swap(y0, y1);
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
        std::swap(x0, x1);
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

  void drawScreenSpaceTriLines(const FragmentType &a, const FragmentType &b, const FragmentType &c) {
    this->drawScreenSpaceLine(a, b);
    this->drawScreenSpaceLine(b, c);
    this->drawScreenSpaceLine(c, a);
  }

  void drawScreenSpaceTriFilled(FragmentType a, FragmentType b, FragmentType c) {
    // Sort our tris by y-coord
    if (a.pos.y > b.pos.y) std::swap(a, b);
    if (b.pos.y > c.pos.y) std::swap(b, c);
    if (a.pos.y > b.pos.y) std::swap(a, b);

    // ok we're sorted now
    fb_pos_t totalHeight = c.pos.y - a.pos.y;
    if (totalHeight == 0) {
      return; // can't draw nuthin
    }
    // bottom half
    fb_pos_t y0 = glm::clamp((fb_pos_t) floor(a.pos.y), 0, heightMax),
      y1 = glm::clamp((fb_pos_t) b.pos.y, 0, heightMax),
      y2 = glm::clamp((fb_pos_t) c.pos.y, 0, heightMax);
    { // top
      //the integer math here fixes tiny triangles going way wrong, for whatever reason?
      float invSlopeA =
        (float) ((fb_pos_t) b.pos.x - (fb_pos_t) a.pos.x)
        / (float) ((fb_pos_t) b.pos.y - (fb_pos_t) a.pos.y);
      float invSlopeB =
        (float) ((fb_pos_t) c.pos.x - (fb_pos_t) a.pos.x)
        / (float) ((fb_pos_t) c.pos.y - (fb_pos_t) a.pos.y);
      if (invSlopeA > invSlopeB) std::swap(invSlopeA, invSlopeB);

      float x0 = a.pos.x;
      float x1 = a.pos.x;
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
      //the integer math here fixes tiny triangles going way wrong, for whatever reason?
      float invSlopeA =
        (float) ((fb_pos_t)c.pos.x - (fb_pos_t)a.pos.x)
        / (float) ((fb_pos_t)c.pos.y - (fb_pos_t)a.pos.y);
      float invSlopeB = (float) ((fb_pos_t)c.pos.x - (fb_pos_t)b.pos.x)
        / (float) ((fb_pos_t)c.pos.y - (fb_pos_t)b.pos.y);
      if (invSlopeA < invSlopeB) std::swap(invSlopeA, invSlopeB);

      float x0 = c.pos.x;
      float x1 = c.pos.x;
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

  // RealSpace(tm) (e.g. clip-space)
  void drawClipSpaceTriangle(const VertexType &a, const VertexType &b, const VertexType &c) {
    // Alrighty we're in clip space, let's transform to screen space first

    float halfW = width / 2.f;
    float halfH = height / 2.f;
    // Z buffer goes here

    // NDC
//  vec3 aa = a / a.w;
//  vec3 bb = b / b.w;
//  vec3 cc = c / c.w;

    // screen-space
    FragmentType ai, bi, ci;
    ai.pos = {(a.pos.x / a.pos.w + 1.f) * halfW, (a.pos.y / a.pos.w + 1.f) * halfH, a.pos.z / a.pos.w, 1};
    bi.pos = {(b.pos.x / b.pos.w + 1.f) * halfW, (b.pos.y / b.pos.w + 1.f) * halfH, a.pos.z / a.pos.w, 1};
    ci.pos = {(c.pos.x / c.pos.w + 1.f) * halfW, (c.pos.y / c.pos.w + 1.f) * halfH, a.pos.z / a.pos.w, 1};

//  cout << to_string(a) << "->" << to_string(ai) << endl;
//  cout << to_string(b) << "->" << to_string(bi) << endl;
//  cout << to_string(c) << "->" << to_string(ci) << endl;

    drawScreenSpaceTriFilled(ai, bi, ci);

//  cout << endl;

  }

  void drawClipSpaceIndexed(DrawMode mode, const VertexBuffer<VertexType> &verts, const IndexBuffer &indicies) {
    if (mode == DrawMode::TRAINGLES) {
      for (size_t i = 0, end = indicies.size(); i + 2 < end; i += 3) {
        drawClipSpaceTriangle(
          verts[indicies[i]],
          verts[indicies[i + 1]],
          verts[indicies[i + 2]]
        );
      }
    } else if (mode == DrawMode::TRIANGLE_STRIP) {
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
    } else if (mode == DrawMode::TRIANGLE_FAN) {
      for (size_t i = 1, end = indicies.size(); i + 1 < end; i++) {
        drawClipSpaceTriangle(
          verts[indicies[0]],
          verts[indicies[i]],
          verts[indicies[i + 1]]
        );
      }
    }
  }

  // Regular/transformable space

private:
  color_t currentColor;
  int width;
  int height;
  int widthMax;
  int heightMax;
  framebuffer_t *framebuffer;

  // Helpers
  inline void drawLineLow(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1) {
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

  inline void drawLineHigh(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1) {
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
};

};

#endif //SOFTREND_SOFTRENDERER_HPP
