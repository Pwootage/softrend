#ifndef SOFTREND_BASICVERTEXSHADER_HPP
#define SOFTREND_BASICVERTEXSHADER_HPP

#include "VertexShader.hpp"
#include "VertexTypes.hpp"

namespace softrend {


class BasicVertexShader : public VertexShader<formats::Pos4ColorNormalTex, formats::Pos4ColorNormalTex> {
  glm::mat4 matrix;
public:
  void kernel(const formats::Pos4ColorNormalTex &vert, formats::Pos4ColorNormalTex &out) override;

  void interpolate(const formats::Pos4ColorNormalTex &a, const formats::Pos4ColorNormalTex &b,
                   const formats::Pos4ColorNormalTex &c, const glm::vec3 &barycentric,
                   formats::Pos4ColorNormalTex &out) override;

  void setMVP(glm::mat4 model, glm::mat4 view, glm::mat4 proj);
};

};


#endif //SOFTREND_BASICVERTEXSHADER_HPP
