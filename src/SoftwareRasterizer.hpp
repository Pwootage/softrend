#ifndef SOFTREND_SOFTRENDERER_HPP
#define SOFTREND_SOFTRENDERER_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtx/type_aligned.hpp>
#include "buffers/VertexBuffer.hpp"
#include "buffers/IndexBuffer.hpp"

#define F32_COLOR
#ifdef F32_COLOR
using color_t = glm::aligned_f32vec4;
#else
using color_t = glm::aligned_u8vec4;
#endif

using framebuffer_t = color_t;
using fb_pos_t = int32_t;

class SoftwareRasterizer {
public:

    enum class DrawMode {
        TRAINGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN
    };

    SoftwareRasterizer(int width, int height);

    // basic members
    int getWidth() const;
    int getHeight() const;
    const framebuffer_t *getFramebuffer() const;


    // Methods
    void clear();


    // state (2d)
    color_t getCurrentColor() const;
    void setCurrentColor(color_t currentColor);

    // ScreenSpace
    void drawScreenSpacePixel(const glm::ivec2 &pos, const color_t &rgba);
    void drawScreenSpacePixel(fb_pos_t index, const color_t &rgba);
    void drawScreenSpaceLine(const glm::ivec2 &a, const glm::ivec2 &b);
    void drawScreenSpaceTriLines(const glm::ivec2 &a, const glm::ivec2 &b, const glm::ivec2 &c);
    void drawScreenSpaceTriFilled(glm::ivec2 aIn, glm::ivec2 bIn, glm::ivec2 cIn);

    // RealSpace(tm) (e.g. clip-space)
    void drawClipSpaceTriangle(const glm::vec4 &a, const glm::vec4 &b, const glm::vec4 &c);
    void drawClipSpaceIndexed(DrawMode mode, const VertexBuffer<glm::vec4> &verts, const IndexBuffer &indicies);

private:
    color_t currentColor;
    int width;
    int height;
    int widthMax;
    int heightMax;
    framebuffer_t *framebuffer;

    // Helpers
    void drawLineLow(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1);
    void drawLineHigh(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1);
};


#endif //SOFTREND_SOFTRENDERER_HPP
