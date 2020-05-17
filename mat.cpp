#include "mat.h"

// Y >= 16 and Y <= 235
// U >= 16 and U <= 240
// V >= 16 and V <= 240

// B * 64 = 74 * ( Y - 16 ) + 129 *( U - 128)

// G * 64 = 74 * ( Y - 16 ) - 52 * (V - 128) - 25 * (U - 128)

// R * 64 = 74 * ( Y - 16 ) + 102 * (V - 128)
int yuv420sp2bgr_half(const unsigned char* yuv, const int w, const int h, unsigned char* bgr, const int direction) {
    if (w % 2 == 1 || h % 2 == 1) {
        fprintf(stderr, "w h should be even. \n");
        return -1;
    }

#if __ARM_NEON
    uint16x8_t _u16 = vdup_n_u16(16);
    uint16x8_t _u74 = vdup_n_u16(74);
    int16x8_t _s129 = vdup_n_s16(129);
    int16x8_t _s102 = vdup_n_s16(102);
    int8x8_t _s52 = vdup_n_s8(52);
    int8x8_t _s25 = vdup_n_s8(25);
    uint8x8_t _u128 = vdup_n_u8(128);

    unsigned char* puv = yuv + w * h;
    unsigned char* py0 = yuv, *py1 = yuv + w; 
    const int wstep = w / 16, hstep = h / 2;

    for (int i = 0; i < hstep; ++i) {
        for (int j = 0; j < wstep; ++j) {
            uint8x16_t y0 = vld1q_u8(py0);
            uint8x16_t y1 = vld1q_u8(py1);

            // first 8 Y
            uint16x8_t low = vaddl_u8(vget_low_u8(y0), vget_low_u8(y1));
            uint16x4_t low_sum = vpadd_u16(low);

            // last 8 Y
            uint16x8_t high = vaddl_u8(vget_high_u8(y0), vget_high_u8(y1));
            uint16x4_t high_sum = vpadd_u16(high);

            uint16x8_t y8_sum = vcombine_u16(low_sum, high_sum);
            uint16x8_t y8 = vshrq_n_u16(y8_sum, 2);

            // build: 74 * (Y - 16)
            y8 = vmaxq_u16(y8, _u16);
            y8 = vsubq_u16(y8, _u16);
            y8 = vmuq_u16(y8, _u74);
            int16x8_t y8i = vreinterpretq_s16_u16(y8);


            // prepare uv
            uint8x8x2_t vu = vld2_u8(puv);
            int8x8_t v = vreinterpretq_s8_u8(vsub_u8(vu.val[0], _u128));
            int8x8_t u = vreinterpretq_s8_u8(vsub_u8(vu.val[1], _u128));
            
            // TODO

            py0 += 8;
            py1 += 8;
            puv += 16;
        }
        // next two row 
        py0 = py1;
        py1 = py0 + w;
    }

#else

#endif
    return 0;
}
