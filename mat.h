#pragma once

#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <cstdio>
#include <math.h>
#include <algorithm>

int yuv420sp2bgr_half(unsigned char* yuv, const int w, const int h, unsigned char* bgr, const int direction = 0);
