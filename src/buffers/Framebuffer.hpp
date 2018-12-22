#ifndef SOFTREND_FRAMEBUFFER_HPP
#define SOFTREND_FRAMEBUFFER_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtx/type_aligned.hpp>

namespace softrend {

#define F32_COLOR
#ifdef F32_COLOR
using color_t = glm::aligned_f32vec4;
#else
using color_t = glm::aligned_u8vec4;
#endif

// using framebuffer_t = color_t;
using depthbuffer_t = float;
using fb_vec_t = glm::aligned_ivec2;
using fb_pos_t = fb_vec_t::value_type;

class Framebuffer {
public:
  virtual void putPixel(fb_pos_t pos, color_t color, float depth) = 0;
  virtual void clear(const color_t &clearColor, bool colorBuffer, bool depthBuffer) = 0;

  virtual color_t getPixel(const fb_pos_t &pos) const = 0;
  virtual float getDepth(fb_pos_t pos) const = 0;
  virtual fb_pos_t getWidth() const = 0;
  virtual fb_pos_t getHeight() const = 0;
};

}

#endif //SOFTREND_FRAMEBUFFER_HPP
