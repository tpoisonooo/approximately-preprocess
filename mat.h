#pragma once

#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <cstdio>
#include <math.h>
#include <algorithm>

int yuv420sp2rgb_half(const unsigned char* yuv, const int w, const int h, unsigned char* rgb); 
