#ifndef SOFTREND_VERTEXSHADER_H
#define SOFTREND_VERTEXSHADER_H

#include <cstdint>
#include <glm/vec3.hpp>

// Interpolation:
// a * f_a / w_a + b * f_b / w_b + c * f_c / w_c f
//-----------------------------------------------------
// a / w_a + b / w_b + c / w_c
//f is the attribute value (each component of each UV)
//a,b,c represents the three verts of the triangle
//and the actual variables a,b,c are the barycentric coordinates
//and w comes from clip space coordinates via the projection matrix

//https://www.khronos.org/registry/OpenGL/specs/gl/glspec44.core.pdf

//barycentric coords
//https://media.discordapp.net/attachments/268578965839544320/516486505896607775/unknown.png

template <typename VertexType, typename VertexOutputType>
class VertexShader {
public:
    typedef union {
        float inFloats[sizeof(VertexType)];
        VertexType input;
    } VertexInput;
    typedef union {
        float outFloats[sizeof(VertexType)];
        VertexOutputType output;
    } VertexOutput;

    virtual void kernel(const VertexInput &vert, VertexOutput &out) = 0;

    template <size_t index, const VertexInput &input>
    inline constexpr glm::vec3 &vec3At() {
      static_assert(index + sizeof(glm::vec3) < sizeof(VertexInput))
      return input.inFloats[index];
    }
};

#endif //SOFTREND_VERTEXSHADER_H
