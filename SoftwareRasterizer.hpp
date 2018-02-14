#ifndef SOFTREND_SOFTRENDERER_HPP
#define SOFTREND_SOFTRENDERER_HPP

#include <cstdint>
#include <glm/vec2.hpp>

using color_t = uint32_t;
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
    void drawPixel(const glm::ivec2 &pos, color_t rgba);
    void clear();
    void drawLine(const glm::ivec2 &a, const glm::ivec2 &b);
    void drawTriLines(const glm::ivec2 &a, const glm::ivec2 &b, const glm::ivec2 &c);
    void drawTriFilled(const glm::ivec2 &aIn, const glm::ivec2 &bIn, const glm::ivec2 &cIn);

private:
    color_t currentColor;
    int width;
    int height;
    framebuffer_t *framebuffer;

    // Helpers
    void drawLineLow(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1);
    void drawLineHigh(fb_pos_t x0, fb_pos_t y0, fb_pos_t x1, fb_pos_t y1);
    void fillTriTop(const glm::ivec2 &tvec2, const glm::ivec2 &tvec21, const glm::ivec2 &tvec22);
};


#endif //SOFTREND_SOFTRENDERER_HPP
