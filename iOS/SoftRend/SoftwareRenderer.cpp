//
//  SoftwareRenderer.m
//  SoftRend
//
//  Created by Christopher Freestone on 12/16/18.
//  Copyright © 2018 Pwootage. All rights reserved.
//

#include <chrono>
#include <numeric>

#import "SoftwareRenderer.h"
#import "src/SoftwareRasterizer.hpp"
#import "src/renderTeapot.hpp"
#include "SoftwareRenderer.h"

using namespace std;
using namespace softrend;

uint64_t teapotFrame;
bool softrend_setUp = false;
constexpr int FRAME_AVG_COUNT = 60;
double frameTimes[FRAME_AVG_COUNT];

void softrend_startup(const char *modelPath) {
  if (softrend_setUp) {
    return;
  }
  softrend_setUp = true;
  renderTeapot::init(modelPath);

  for (int i = 0; i < FRAME_AVG_COUNT; i++) {
    frameTimes[i] = 1000.f / 60.f;
  }
}

void softrend_render() {
  auto startT = chrono::steady_clock::now();
  renderTeapot::render(teapotFrame);
  auto endT = chrono::steady_clock::now();
  auto time = chrono::duration<double, milli>(endT - startT).count();
  frameTimes[teapotFrame % FRAME_AVG_COUNT] = time;

  teapotFrame++;
}

void *softrend_getFramebuffer() {
  return (void *) renderTeapot::getFB();
}

uint32_t softrend_getFBWidth() {
  return (uint32_t) renderTeapot::getFBWidth();
}

uint32_t softrend_getFBHeight() {
  return (uint32_t) renderTeapot::getFBHeight();
}

float softrend_averageTiming() {
  auto avg = accumulate(
    begin(frameTimes),
    end(frameTimes),
    0.0
  ) / (double) FRAME_AVG_COUNT;
  return (float) avg;
}

int softrend_getFrame() {
  return (int) teapotFrame;
}
