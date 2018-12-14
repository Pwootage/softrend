#ifndef SOFTREND_FRAGMENTSHADER_H
#define SOFTREND_FRAGMENTSHADER_H

#include <cstdint>
#include <glm/vec4.hpp>

namespace softrend {

template<typename FragmentType>
class FragmentShader {
public:

  virtual bool kernel(const FragmentType &frag, bool frontFacing, glm::vec4 &out_color, float &outDepth) = 0;
};

};

#endif //SOFTREND_FRAGMENTSHADER_H
