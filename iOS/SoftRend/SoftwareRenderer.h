//
//  SoftwareRenderer.h
//  SoftRend
//
//  Created by Christopher Freestone on 12/16/18.
//  Copyright © 2018 Pwootage. All rights reserved.
//

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t frame;
void softrend_startup(const char* modelPath);
void softrend_render();
void *softrend_getFramebuffer();
uint32_t softrend_getFBWidth();
uint32_t softrend_getFBHeight();
float softrend_averageTiming();
int softrend_getFrame();

#ifdef __cplusplus
};
#endif
