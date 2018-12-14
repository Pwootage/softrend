#include "PhongFragmentShader.hpp"

using namespace glm;
using namespace softrend;

bool PhongFragmentShader::kernel(const formats::Pos4ColorNormalTex &frag,
                                 bool frontFacing,
                                 glm::vec4 &out_color,
                                 float &outDepth) {
  float diffuseIntensity = min(dot(frag.normal, light_dir), 1.f);

  out_color = frag.color * light_color_ambient
              + frag.color * light_color_diffuse * diffuseIntensity;
  outDepth = frag.pos.z;
  return true;
}
