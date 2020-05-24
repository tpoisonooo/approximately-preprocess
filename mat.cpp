#include "mat.h"

int yuv420sp2bgr_half(unsigned char* yuv, const int w, const int h, unsigned char* bgr, const int direction) {
    if (w % 2 == 1 || h % 2 == 1) {
        fprintf(stderr, "w h should be even. \n");
        return -1;
    }

#if __ARM_NEON
    uint8x8_t _u128 = vdup_n_u8(128);
    int8x8_t _s90 = vdup_n_s8(90);
    int8x8_t _sn46 = vdup_n_s8(-46);
    int8x8_t _s113 = vdup_n_s8(113);
    int8x8_t _sn22 = vdup_n_s8(-22);
    int16x8_t _s0 = vdupq_n_s16(0);
    int16x8_t _s8160 = vdupq_n_s16(8160); // 255 << 6
#endif

    unsigned char* puv = yuv + w * h;
    unsigned char* py0 = yuv, *py1 = yuv + w; 
    const int wstep = w / 16, hstep = h / 2;

    for (int i = 0; i < hstep; ++i) {
        for (int j = 0; j < wstep; ++j) {
#if __ARM_NEON
            uint8x16_t y0 = vld1q_u8(py0);
            uint8x16_t y1 = vld1q_u8(py1);

            // first 8 Y
            uint16x8_t low = vaddl_u8(vget_low_u8(y0), vget_low_u8(y1));
            uint16x4_t low_sum = vpadd_u16(vget_low_u16(low), vget_high_u16(low));

            // last 8 Y
            uint16x8_t high = vaddl_u8(vget_high_u8(y0), vget_high_u8(y1));
            uint16x4_t high_sum = vpadd_u16(vget_low_u16(high), vget_high_u16(high));

            uint16x8_t y8_sum = vcombine_u16(low_sum, high_sum);
            // y8 = (y8_sum >> 2) << 6 = y8_sum << 4;
            uint16x8_t y8 = vshlq_n_u16(y8_sum, 4);

            // prepare uv
            uint8x8x2_t vu = vld2_u8(puv);
            int8x8_t v = vreinterpret_s8_u8(vsub_u8(vu.val[0], _u128));
            int8x8_t u = vreinterpret_s8_u8(vsub_u8(vu.val[1], _u128));
            
            int16x8_t r_acc = vmlal_s8(y8, v, _s90);
            int16x8_t g_acc = vmlal_s8(y8, v, _sn46);
            g_acc = vmlal_s8(g_acc, u, _sn22);
            int16x8_t b_acc = vmlal_s8(y8, u, _s113);

#define SHIFT_6_SATURATE(FROM,TO) \
            FROM = vmaxq_s16(vminq_s16((FROM), _s8160), _s0); \
            uint8x8_t TO = vshrn_n_s16(vreinterpretq_u16_s16((FROM)), 6);
            
            SHIFT_6_SATURATE(b_acc, b_out)
            SHIFT_6_SATURATE(g_acc, g_out)
            SHIFT_6_SATURATE(r_acc, r_out)
#undef SHIFT_6_SATURATE

            uint8x8x3_t _bgr;
            _bgr.val[0] = b_out;
            _bgr.val[1] = g_out;
            _bgr.val[2] = r_out;
            if (direction == 0) {
               vst3_u8(bgr, _bgr); 
               bgr += 24;
            } else {
                fprintf(stderr, "not implemented direction %d\n", direction);
            }

#else
            for (int idx = 0; idx < 8; ++idx) {
                int y = (static_cast<int>(py0[idx * 2]) + py0[idx * 2 + 1] + py1[idx * 2] + py1[idx * 2 + 1]) << 4;
                int v = static_cast<int>(puv[idx * 2]) - 128;
                int u = static_cast<int>(puv[idx * 2 + 1]) - 128;

                int ruv = 90 * v;
                int guv = -46 * v + -22 * u;
                int buv = 113 * u;

#define SATURATE_CAST_UCHAR(X) (unsigned char)::std::min(::std::max((int)(X), 0), 255);
                bgr[idx * 3] = SATURATE_CAST_UCHAR((y + buv) >> 6);
                bgr[idx * 3 + 1] = SATURATE_CAST_UCHAR((y + guv) >> 6);
                bgr[idx * 3 + 2] = SATURATE_CAST_UCHAR((y + ruv) >> 6);
            }
            bgr += 24;
#endif

            py0 += 16;
            py1 += 16;
            puv += 16;
        }
        // next two row 
        py0 = py1;
        py1 = py0 + w;
    }
    return 0;
}
