#ifndef SOFTREND_FRAMEBUFFER_HPP
#define SOFTREND_FRAMEBUFFER_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtx/type_aligned.hpp>

namespace softrend {

// #define F32_COLOR
// #ifdef F32_COLOR
// using color_t = glm::aligned_f32vec4;
// #else
// using color_t = glm::aligned_u8vec4;
// #endif

using f32_color_t __attribute__((aligned(16))) =
glm::aligned_f32vec4;
using u8_color_t __attribute__((aligned(4))) =
glm::aligned_u8vec4;

namespace color_conversion {
template<typename color_t>
struct no_convert {
  const color_t &operator()(const color_t &color) const {
    return color;
  };
};

inline float u8_to_f32(uint8_t value) {
  return (float) value / 255.f;
}

inline uint8_t f32_to_u8(float value) {
  return (uint8_t)(glm::clamp<float>(value, 0, 1) * 255.f);
//  return (uint8_t)(value * 255.f);
}

struct u8_f32_convert {
  f32_color_t operator()(const u8_color_t &color) const {
    return {
        u8_to_f32(color.r),
        u8_to_f32(color.g),
        u8_to_f32(color.b),
        u8_to_f32(color.a)
    };
  }
};

struct f32_u8_convert {
  u8_color_t operator()(const f32_color_t &color) const {
    return {
        f32_to_u8(color.r),
        f32_to_u8(color.g),
        f32_to_u8(color.b),
        f32_to_u8(color.a)
    };
  }
};
}


// using framebuffer_t = color_t;
using depthbuffer_t = float;
using fb_vec_t = glm::aligned_ivec2;
using fb_pos_t = fb_vec_t::value_type;

template<typename color_t>
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
