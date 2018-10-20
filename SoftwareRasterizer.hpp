#ifndef SOFTREND_SOFTRENDERER_HPP
#define SOFTREND_SOFTRENDERER_HPP

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>

#define F32_COLOR
#ifdef F32_COLOR
using color_t = glm::f32vec4;
#else
using color_t = glm::vec<4, uint8_t, glm::aligned_lowp>;
#endif

using framebuffer_t = color_t;
using fb_pos_t = int32_t;

class SoftwareRasterizer {
public:

    SoftwareRasterizer(int width, int height);

    // basic members
    int getWidth() const;
    int getHeight() const;
    const framebuffer_t *getFramebuffer() const;

    // state
    color_t getCurrentColor() const;
    void setCurrentColor(color_t currentColor);

    // Methods
    void drawPixel(const glm::ivec2 &pos, const color_t &rgba);
    void drawPixel(fb_pos_t index, const color_t &rgba);
    void clear();
    void drawLine(const glm::ivec2 &a, const glm::ivec2 &b);
    void drawTriLines(const glm::ivec2 &a, const glm::ivec2 &b, const glm::ivec2 &c);
    void drawTriFilled(glm::ivec2 aIn, glm::ivec2 bIn, glm::ivec2 cIn);

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
