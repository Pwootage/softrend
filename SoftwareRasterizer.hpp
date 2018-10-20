#ifndef SOFTREND_SOFTRENDERER_HPP
#define SOFTREND_SOFTRENDERER_HPP

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>

using color_t = glm::u8vec4;
//struct alignas(4) color_t {
//  uint8_t r;
//  uint8_t g;
//  uint8_t b;
//  uint8_t a;
//};

//struct alignas(4) color_t {
//    inline color_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : color(r, g, b, a) {
//    }
//    inline color_t() : color_t(0,0,0,0) {
//    }
//
//    union {
//        uint32_t align;
//        glm::u8vec4 color;
//    };
//};
//
//struct alignas(4) framebuffer_t {
//    union {
//        uint32_t align;
//        glm::u8vec4 color;
//    };
//};

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
