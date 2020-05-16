#ifndef SOFTREND_BASICFRAGMENTSHADER_HPP
#define SOFTREND_BASICFRAGMENTSHADER_HPP

#include "softrend/VertexTypes.hpp"
#include "FragmentShader.hpp"

namespace softrend {

class PhongFragmentShader : public FragmentShader<formats::Pos4ColorNormalTex> {
public:
  glm::vec3 light_dir;
  glm::vec4 light_color_ambient{0.3f, 0.3f, 0.3f, 1.f};
  glm::vec4 light_color_diffuse{0.6f, 0.6f, 0.6f, 1.f};

  bool kernel(const formats::Pos4ColorNormalTex &frag, bool frontFacing, glm::vec4 &out_color, float &outDepth) override;
};


}


#endif //SOFTREND_BASICFRAGMENTSHADER_HPP
