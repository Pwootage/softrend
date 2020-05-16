#ifndef SOFTREND_VERTEXTYPES_HPP
#define SOFTREND_VERTEXTYPES_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/type_aligned.hpp>

namespace softrend::formats {

struct Pos4ColorNormalTex {
  glm::aligned_vec4 pos;
  glm::aligned_vec4 color;
  glm::aligned_vec3 normal;
  glm::aligned_vec2 uv;
};

template<size_t tex_count>
struct Pos4ColorNormalMultiTex {
  glm::aligned_vec4 pos;
  glm::aligned_vec4 color;
  glm::aligned_vec3 normal;
  glm::aligned_vec2 uv[tex_count];
};

}

#endif //SOFTREND_VERTEXTYPES_HPP
