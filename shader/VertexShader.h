#ifndef SOFTREND_VERTEXSHADER_H
#define SOFTREND_VERTEXSHADER_H

#include <cstdint>

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


template <typename inData, typename outData>
class VertexShader {
public:
    typedef union {
        float inFloats[sizeof(inData)];
        inData input;
    } vertexInput;
    typedef union {

        float outFloats[sizeof(inData)];
        inData output;
    } vertexOutput;


};

#endif //SOFTREND_VERTEXSHADER_H
