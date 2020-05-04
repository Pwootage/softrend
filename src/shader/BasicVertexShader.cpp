#include "BasicVertexShader.hpp"
#include <iostream>

using namespace softrend;
using namespace glm;

void BasicVertexShader::kernel(const formats::Pos4ColorNormalTex &vert, formats::Pos4ColorNormalTex &out) {
  out.pos = matrix * vert.pos;
  out.color = vert.color;
  out.normal = vert.normal;
  out.uv = vert.uv;
}

void BasicVertexShader::interpolate(const formats::Pos4ColorNormalTex &a,
                                    const formats::Pos4ColorNormalTex &b,
                                    const formats::Pos4ColorNormalTex &c,
                                    const glm::vec3 &barycentric,
                                    formats::Pos4ColorNormalTex &out) {
  out.pos = a.pos * barycentric.x + b.pos * barycentric.y + c.pos * barycentric.z;
  out.color = a.color * barycentric.x + b.color * barycentric.y + c.color * barycentric.z;
  // out.color = a.color;
  out.normal = a.normal * barycentric.x + b.normal * barycentric.y + c.normal * barycentric.z;
  // out.uv = a.uv * barycentric.x + b.uv * barycentric.y + c.uv * barycentric.z;
}


void BasicVertexShader::setMVP(glm::mat4 model, glm::mat4 view, glm::mat4 proj) {
  matrix = proj * view * model;
}

