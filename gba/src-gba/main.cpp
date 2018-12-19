#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <stdio.h>
#include <chrono>
#include <numeric>

#include "../src/SoftwareRasterizer.hpp"
#include "../src/renderTeapot.hpp"
#include "teapot_low_obj.h"

using namespace std;

double s = 0;
double lastStart = 0;

size_t frame = 0;
constexpr int FRAME_AVG_COUNT = 60;
double frameTimes[FRAME_AVG_COUNT];

void timerHandler() {
  s += 0.001;
}

void updateTimerText() {
  int seconds_int = s;
  int minutes = (seconds_int / 60) % 60;
  int hours = (seconds_int / 60 / 60);

  auto avg = accumulate(
    begin(frameTimes),
    end(frameTimes),
    0.0
  ) / (double) FRAME_AVG_COUNT;
  printf("\x1b[12;00HAvg %.2lfs    ", avg);

  printf("\x1b[13;00Hrun time: %dh %02dm %02.2lfs    ", hours, minutes, glm::mod(s, 60.));
}

int main(void) {
//---------------------------------------------------------------------------------

  // the vblank interrupt must be enabled for VBlankIntrWait() to work
  // since the default dispatcher handles the bios flags no vblank handler
  // is required
  irqInit();
  irqEnable(IRQ_VBLANK);
  irqEnable(IRQ_TIMER2);
  // irqEnable(IRQ_TIMER3);
  irqSet(IRQ_TIMER2, &timerHandler);
  // irqSet(IRQ_TIMER3, &updateTimerText);

  consoleDemoInit();

  renderTeapot::InitData initData;
  initData.modelSrc = (const char*)teapot_low_obj;
  initData.modelLen = teapot_low_obj_size;
  // initData.fb_width = 16;
  // initData.fb_height = 16;

  renderTeapot::init(initData);

  // ansi escape sequence to clear screen and home cursor
  // /x1b[line;columnH
  printf("\x1b[2J");

  for (int i = 0; i < FRAME_AVG_COUNT; i++) {
    frameTimes[i] = 1.;
  }

  // set up some timer registeres
  // frame timing: 280896 cycles per frame
  // 280896/64 = 4389
  // REG_TM3CNT_L = -4389;
  // REG_TM3CNT_H = TIMER_IRQ | TIMER_START | 1; // 1 = 64 cycles

  // 2^24 hz, 2^24/1000 = 16777.216 ~= 0x4189 cycles per ms
  REG_TM2CNT_L = -0x4189; // 1 << 24 hz / (1024/2^10) = 2^14  = 0x4000 per sec, / 1000c
  REG_TM2CNT_H = TIMER_IRQ | TIMER_START | 0; // 0 = 1 cycle


  while (1) {
    VBlankIntrWait();
    printf("\x1b[9;0H%u start", frame);

    lastStart = s;
    auto startT = s;
    renderTeapot::render(frame);
    auto endT = s;
    auto time = endT - startT;
    frameTimes[frame % FRAME_AVG_COUNT] = time;

    printf("\x1b[10;0H%u end", frame);
    printf("\x1b[11;00HFrame time: %.2lfs    ", time);

    updateTimerText();


    frame++;
  }
}

