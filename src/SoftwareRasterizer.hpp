#ifndef SOFTREND_SOFTRENDERER_HPP
#define SOFTREND_SOFTRENDERER_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>
#include <limits>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtx/type_aligned.hpp>
#include "buffers/VertexBuffer.hpp"
#include "buffers/IndexBuffer.hpp"
#include "shader/VertexShader.hpp"
#include "shader/FragmentShader.hpp"

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

namespace softrend {

#define F32_COLOR
#ifdef F32_COLOR
using color_t = glm::aligned_f32vec4;
#else
using color_t = glm::aligned_u8vec4;
#endif

using framebuffer_t = color_t;
using depthbuffer_t = float;
using fb_vec_t = glm::aligned_ivec2;
using fb_pos_t = fb_vec_t::value_type;

enum class DrawMode {
  TRAINGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN
};

enum class CullMode {
  FRONT_FACING,
  BACK_FACING,
  NONE,
  ALL
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

    this->framebuffer = static_cast<framebuffer_t *>(malloc(sizeof(framebuffer_t) * width * height));
    this->depthbuffer = static_cast<depthbuffer_t *>(malloc(sizeof(depthbuffer_t) * width * height));
  }

  // basic members
  size_t getWidth() const {
    return width;
  }

  size_t getHeight() const {
    return height;
  }

  const framebuffer_t *getFramebuffer() const {
    return framebuffer;
  }

  // Methods
  void clear() {
    float clearDepth = std::numeric_limits<float>::infinity();
    for (fb_pos_t i = 0; i < width * height; i++) {
      this->framebuffer[i] = this->currentColor;
      this->depthbuffer[i] = clearDepth;
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
  inline void drawScreenSpacePixel(fb_pos_t idx, const color_t &rgba, float depth) {
    if (depthbuffer[idx] <= depth) return;
    depthbuffer[idx] = depth;

//  this->framebuffer[idx].align = rgba.align;
//  this->framebuffer[idx] = rgba;
// we take a block here depending on what's defined
#ifdef __SSE2__
#ifdef F32_COLOR
    *reinterpret_cast<__m128i *>(&this->framebuffer[idx]) = *reinterpret_cast<const __m128i *>(&rgba);
#else
    *reinterpret_cast<uint32_t *>(&this->framebuffer[idx]) = *reinterpret_cast<const uint32_t *>(&rgba);
#endif
#elif __ARM_NEON
    *reinterpret_cast<float32x4_t *>(&this->framebuffer[idx]) = *reinterpret_cast<const float32x4_t *>(&rgba);
#else
    this->framebuffer[idx] = rgba;
#endif
  }

  void drawScreenSpaceLine(const VertexType &a, const VertexType &b) {
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

  void drawScreenSpaceTriLines(const VertexType &a, const VertexType &b, const VertexType &c) {
    this->drawScreenSpaceLine(a, b);
    this->drawScreenSpaceLine(b, c);
    this->drawScreenSpaceLine(c, a);
  }

private:
  inline fb_pos_t orient2D(const fb_vec_t &a, const fb_vec_t &b, const fb_vec_t &c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }

public:
  void drawScreenSpaceTriFilled(FragmentType aFrag, FragmentType bFrag, FragmentType cFrag) {
    constexpr fb_pos_t subPixelScale = 4;
    constexpr fb_pos_t subPixelStep = 1 << subPixelScale; // 4 bits
    constexpr fb_pos_t subPixelMask = subPixelStep - 1; // 4 bits

    fb_vec_t a = {aFrag.pos.x * subPixelStep, aFrag.pos.y * subPixelStep};
    fb_vec_t b = {bFrag.pos.x * subPixelStep, bFrag.pos.y * subPixelStep};
    fb_vec_t c = {cFrag.pos.x * subPixelStep, cFrag.pos.y * subPixelStep};

    fb_pos_t det = orient2D(a, b, c);
    float detF = det;

    bool frontFacing = det > 0;

    switch (cullMode) {
      case CullMode::FRONT_FACING:
        if (frontFacing) return;
        break;
      case CullMode::BACK_FACING:
        if (!frontFacing) return;
        break;
      case CullMode::ALL:
        return;
      case CullMode::NONE:
        break;
    }

    fb_vec_t min = glm::min(glm::min(a, b), c);
    fb_vec_t max = glm::max(glm::max(a, b), c);
    min = glm::clamp(min, {0, 0}, {widthMax << subPixelScale, heightMax << subPixelScale});
    max = glm::clamp(max, {0, 0}, {widthMax << subPixelScale, heightMax << subPixelScale});

    min.x = (min.x + subPixelStep) & ~subPixelMask;
    min.y = (min.y + subPixelStep) & ~subPixelMask;

    // Slopes of the various barycentric coordinates
    // Need to go up by subPixelStep per row, but that's a shift so it's fast
    fb_pos_t A01 = (a.y - b.y) << subPixelScale, B01 = (b.x - a.x) << subPixelScale;
    fb_pos_t A12 = (b.y - c.y) << subPixelScale, B12 = (c.x - b.x) << subPixelScale;
    fb_pos_t A20 = (c.y - a.y) << subPixelScale, B20 = (a.x - c.x) << subPixelScale;

    // Barycentric stepping setup
    fb_vec_t p = min;
    VertexType interpolated;

    fb_pos_t aRow = orient2D(b, c, p);
    fb_pos_t bRow = orient2D(c, a, p);
    fb_pos_t cRow = orient2D(a, b, p);

    for (p.y = min.y; p.y <= max.y; p.y += subPixelStep) {
      fb_pos_t yOff = (p.y >> subPixelScale) * width;
      fb_pos_t aBary = aRow;
      fb_pos_t bBary = bRow;
      fb_pos_t cBary = cRow;

      for (p.x = min.x; p.x <= max.x; p.x += subPixelStep) {
        if ((aBary | bBary | cBary) >= 0) {
          glm::vec3 bary;
          bary.x = (float) aBary / detF;
          bary.y = (float) bBary / detF;
          bary.z = (float) cBary / detF;

          vertexShader->interpolate(aFrag, bFrag, cFrag, bary, interpolated);

          float depth;
          color_t color;
          if (fragmentShader->kernel(interpolated, false, color, depth)) {
            drawScreenSpacePixel(yOff + (p.x >> subPixelScale), color, depth);
          }
        }

        aBary += A12;
        bBary += A20;
        cBary += A01;
      }

      aRow += B12;
      bRow += B20;
      cRow += B01;
    }
  }

  // RealSpace(tm) (e.g. clip-space)
  void drawClipSpaceTriangle(const FragmentType &a, const FragmentType &b, const FragmentType &c) {
    // Alrighty we're in clip space, let's transform to screen space first

    float halfW = width / 2.f;
    float halfH = height / 2.f;
    // Z buffer goes here

    // NDC
//  vec3 aa = a / a.w;
//  vec3 bb = b / b.w;
//  vec3 cc = c / c.w;

    // screen-space
    FragmentType ai = a, bi = b, ci = c;
    ai.pos = {(a.pos.x / a.pos.w + 1.f) * halfW, (a.pos.y / a.pos.w + 1.f) * halfH, a.pos.z / a.pos.w, a.pos.w};
    bi.pos = {(b.pos.x / b.pos.w + 1.f) * halfW, (b.pos.y / b.pos.w + 1.f) * halfH, b.pos.z / b.pos.w, b.pos.w};
    ci.pos = {(c.pos.x / c.pos.w + 1.f) * halfW, (c.pos.y / c.pos.w + 1.f) * halfH, c.pos.z / c.pos.w, c.pos.w};

//  cout << to_string(a) << "->" << to_string(ai) << endl;
//  cout << to_string(b) << "->" << to_string(bi) << endl;
//  cout << to_string(c) << "->" << to_string(ci) << endl;

    drawScreenSpaceTriFilled(ai, bi, ci);

//  cout << endl;

  }

  void drawClipSpaceIndexed(DrawMode mode, const VertexBuffer<FragmentType> &verts, const IndexBuffer &indicies) {
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

  void drawTriangle(const VertexType &a, const VertexType &b, const VertexType &c) {
    FragmentType fragA, fragB, fragC;

    vertexShader->kernel(a, fragA);
    vertexShader->kernel(b, fragB);
    vertexShader->kernel(c, fragC);

    drawClipSpaceTriangle(fragA, fragB, fragC);
  }

  void drawIndexed(DrawMode mode, const VertexBuffer<VertexType> &verts, const IndexBuffer &indicies) {
    if (mode == DrawMode::TRAINGLES) {
      for (size_t i = 0, end = indicies.size(); i + 2 < end; i += 3) {
        drawTriangle(
          verts[indicies[i]],
          verts[indicies[i + 1]],
          verts[indicies[i + 2]]
        );
      }
    } else if (mode == DrawMode::TRIANGLE_STRIP) {
      for (size_t i = 0, end = indicies.size(); i + 2 < end; i++) {

        if (i % 2 == 0) {
          drawTriangle(
            verts[indicies[i]],
            verts[indicies[i + 1]],
            verts[indicies[i + 2]]
          );
        } else {
          drawTriangle(
            verts[indicies[i + 2]],
            verts[indicies[i + 1]],
            verts[indicies[i]]
          );
        }
      }
    } else if (mode == DrawMode::TRIANGLE_FAN) {
      for (size_t i = 1, end = indicies.size(); i + 1 < end; i++) {
        drawTriangle(
          verts[indicies[0]],
          verts[indicies[i]],
          verts[indicies[i + 1]]
        );
      }
    }
  }

  VertexShader<VertexType, FragmentType> *vertexShader;
  FragmentShader<FragmentType> *fragmentShader;
private:
  color_t currentColor;
  size_t width;
  size_t height;
  size_t widthMax;
  size_t heightMax;
  framebuffer_t *framebuffer;
  depthbuffer_t *depthbuffer;
  CullMode cullMode = CullMode::BACK_FACING;

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
