#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <stdio.h>
#include <chrono>
#include <numeric>
#include <sstream>
#include <iomanip>

#include "../src/SoftwareRasterizer.hpp"
#include "../src/renderTeapot.hpp"
#include "GBAMode3Framebuffer.hpp"
#include "teapot_low_obj.h"
#include "amiga_fnt.h"

using namespace std;
using namespace softrend;

constexpr double MS_PER_TICK = 0.000999987125;
size_t tick = 0;

size_t frame = 0;
constexpr int FRAME_AVG_COUNT = 60;
double frameTimes[FRAME_AVG_COUNT];

void timerHandler() {
  tick++;
}

void drawChar(int x, int y, char c) {
  int offset = c * 8;
  for (int y1 = 0; y1 < 8; y1++) {
    u8 charLine = amiga_fnt[offset + y1];
    for (int x1 = 0; x1 < 8; x1++) {
      bool set = (charLine & (1 << (7 - x1))) > 0;
      u16 color = set ? (u16) 0x1f : (u16) 0;
      MODE3_FB[y1 + y][x1 + x] = RGB5(color, color, color);
    }
  }
}

void updateTimerText() {
  double s = MS_PER_TICK * tick;
  int seconds_int = static_cast<int>(s);
  int minutes = (seconds_int / 60) % 60;
  int hours = (seconds_int / 60 / 60);

  auto avg = accumulate(
    begin(frameTimes),
    end(frameTimes),
    0.0
  ) / (double) FRAME_AVG_COUNT;

  {
    stringstream ss;
    ss << setprecision(4);
    ss << "Avg " << avg << "s";
    ss << " Frame #" << frame;
    string str = ss.str();

    int x = 0;
    int y = 160 - 8;
    for (size_t i = 0; i < str.size(); i++) {
      drawChar(x, y, str[i]);
      x += 8;
      if (x > 240) x = 0;
    }
  }

  {
    stringstream ss;
    ss << setprecision(4);
    ss << "Run time: ";
    ss << hours << "h ";
    ss << minutes << "m ";
    ss << glm::mod(s, 60.) << "s";

    string str = ss.str();

    int x = 0;
    int y = 160 - 8 * 2;
    for (size_t i = 0; i < str.size(); i++) {
      drawChar(x, y, str[i]);
      x += 8;
      if (x > 240) x = 0;
    }
  }
}

GBAMode3Framebuffer framebuffer;

int main(void) {
//---------------------------------------------------------------------------------

  // the vblank interrupt must be enabled for VBlankIntrWait() to work
  // since the default dispatcher handles the bios flags no vblank handler
  // is required
  irqInit();
  // irqEnable(IRQ_VBLANK);
  irqEnable(IRQ_TIMER2);
  irqSet(IRQ_TIMER2, &timerHandler);

  SetMode(MODE_3 | BG2_ON);
  // consoleDemoInit();

  renderTeapot::InitData initData;
  initData.modelSrc = (const char *) teapot_low_obj;
  initData.modelLen = teapot_low_obj_size;
  initData.framebuffer = &framebuffer;

  renderTeapot::init(initData);

  // ansi escape sequence to clear screen and home cursor
  // /x1b[line;columnH
  printf("\x1b[2J");

  for (int i = 0; i < FRAME_AVG_COUNT; i++) {
    frameTimes[i] = 1.;
  }

  // 2^24 hz, 2^24/1000 = 16777.216 ~= 0x4189 cycles per ms
  REG_TM2CNT_L = -0x4189; // 1 << 24 hz / (1024/2^10) = 2^14  = 0x4000 per sec, / 1000c
  REG_TM2CNT_H = TIMER_IRQ | TIMER_START | 0; // 0 = 1 cycle

  while (1) {
    // VBlankIntrWait();
    framebuffer.clear({0,0,0,0}, true, true);

    updateTimerText();

    auto startT = tick;
    renderTeapot::render(frame);
    auto endT = tick;
    auto time = endT - startT;
    frameTimes[frame % FRAME_AVG_COUNT] = time * MS_PER_TICK;

    frame++;
  }
}

