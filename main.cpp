#include <iostream>
#include <opencv/cv.hpp>
#include <cmath>
#include <sys/time.h>

unsigned char clip(unsigned char val) {
    int v = val;
    return (unsigned char)std::min(255, std::max(0, v));
}

void BGR2YUV420P(unsigned char *bgr24, int width, int height, unsigned char *yuv420p) {
    unsigned char *ptrY, *ptrU, *ptrV;
    unsigned char *ptrVU;
    memset(yuv420p, 0, width * height * 3 / 2);
    ptrY = yuv420p;
    ptrU = yuv420p + width * height;
    ptrVU = yuv420p + width * height;
    ptrV = ptrU + (width * height * 1 / 4);
    unsigned char y, u, v, r, g, b;
    int index = 0;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            index = width * j * 3 + i * 3;
            b = bgr24[index];
            g = bgr24[index + 1];
            r = bgr24[index + 2];
            y = (unsigned char) ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = (unsigned char) ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = (unsigned char) ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            *(ptrY++) = clip(y);

           // if (j % 2 == 0 && i % 2 == 0) {
           //     *(ptrU++) = clip(u);
           // } else if (i % 2 == 0) {
           //     *(ptrV++) = clip(v);
           // }
            if (j % 2 == 0 && i % 2 == 0) {
                *(ptrVU++) = clip(v);
                *(ptrVU++) = clip(u);
            }
        }
    }
}


void BGR2NV12v1(const unsigned char* bgr, const int w, const int h, unsigned char* nv12, const int out_w, const int out_h) {
    assert(out_w % 2 == 0);
    assert(out_w >= w and out_h >= h);
    const int AREA = out_w * out_h;
    unsigned char* base_uv = nv12 + AREA;
    memset(nv12, 0, AREA);
    // black YUV
    memset(nv12 + AREA, 128, AREA >> 1);
    unsigned char y, u, v, r, g, b;

    for (int j = 0; j < h; j++) {
        const int base = w * j * 3;
        unsigned char* ptrY = nv12 + out_w * j;
        unsigned char* ptrUV  = base_uv + out_w / 2 * j;
        for (int i = 0; i < w; i++) {
            const int index = base + i * 3;
            b = bgr[index];
            g = bgr[index + 1];
            r = bgr[index + 2];

            *(ptrY++) =  (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;

#define CLIP(x) ((unsigned char)std::min(255, std::max(0, static_cast<int>(x))))
            if (j % 2 == 0 and i % 2 == 0) {
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u);
                *(ptrUV++) = CLIP(v);
            }
#undef CLIP
        }
    }
}

void BGR2NV12v2(const unsigned char* bgr, const int w, const int h, unsigned char* nv12, const int out_w, const int out_h) {
    assert(out_w % 2 == 0);
    assert(out_w >= w and out_h >= h);
    const int AREA = out_w * out_h;
    unsigned char* base_uv = nv12 + AREA;
    // black YUV
    memset(nv12, 0, AREA);
    memset(nv12 + AREA, 128, AREA >> 1);

    for (int j = 0; j < h; j++) {
        const int base = w * j * 3;
        unsigned char* ptrY = nv12 + out_w * j;
        unsigned char* ptrUV  = base_uv + out_w / 2 * j;

        const int w_step = w >> 2;
        int i = 0;
        for (; i < w_step; i++) {
            const int index = base + i * 12;
            const unsigned char b0 = bgr[index], g0 = bgr[index + 1], r0 = bgr[index + 2];
            const unsigned char b1 = bgr[index + 3], g1 = bgr[index + 4], r1 = bgr[index + 5];
            const unsigned char b2 = bgr[index + 6], g2 = bgr[index + 7], r2 = bgr[index + 8];
            const unsigned char b3 = bgr[index + 9], g3 = bgr[index + 10], r3 = bgr[index + 11];

            *(ptrY++) =  (unsigned char)((66 * r0 + 129 * g0 + 25 * b0 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r1 + 129 * g1 + 25 * b1 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r2 + 129 * g2 + 25 * b2 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r3 + 129 * g3 + 25 * b3 + 128) >> 8) + 16;

#define CLIP(x) ((unsigned char)std::min(255, std::max(0, static_cast<int>(x))))
            if (j % 2 == 0) {
                const unsigned char u0 = (unsigned char)((-38 * r0 - 74 * g0 + 112 * b0 + 128) >> 8) + 128;
                const unsigned char v0 = (unsigned char)((112 * r0 - 94 * g0 - 18 * b0 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u0);
                *(ptrUV++) = CLIP(v0);

                const unsigned char u2 = (unsigned char)((-38 * r2 - 74 * g2 + 112 * b2 + 128) >> 8) + 128;
                const unsigned char v2 = (unsigned char)((112 * r2 - 94 * g2 - 18 * b2 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u2);
                *(ptrUV++) = CLIP(v2);
            }
#undef CLIP
        }

        i = w_step << 2;
        unsigned char y, u, v, r, g, b;
        for (; i < w; i++) {
            const int index = base + i * 3;
            b = bgr[index];
            g = bgr[index + 1];
            r = bgr[index + 2];

            *(ptrY++) =  (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;

#define CLIP(x) ((unsigned char)std::min(255, std::max(0, static_cast<int>(x))))
            if (j % 2 == 0 and i % 2 == 0) {
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u);
                *(ptrUV++) = CLIP(v);
            }
#undef CLIP
        }
    }
}

void BGR2NV12v3(const unsigned char* bgr, const int w, const int h, unsigned char* nv12, const int out_w, const int out_h) {
    assert(out_w % 2 == 0);
    assert(out_w >= w and out_h >= h);
    const int AREA = out_w * out_h;
    unsigned char* base_uv = nv12 + AREA;
    // black YUV
    memset(nv12, 0, AREA);
    memset(nv12 + AREA, 128, AREA >> 1);

    for (int j = 0; j < h; j++) {
        const int base = w * j * 3;
        unsigned char* ptrY = nv12 + out_w * j;
        unsigned char* ptrUV  = base_uv + out_w / 2 * j;

        const int w_step = w >> 3;
        int i = 0;
        for (; i < w_step; i++) {
            const int index = base + i * 24;
            const unsigned char b0 = bgr[index], g0 = bgr[index + 1], r0 = bgr[index + 2];
            const unsigned char b1 = bgr[index + 3], g1 = bgr[index + 4], r1 = bgr[index + 5];
            const unsigned char b2 = bgr[index + 6], g2 = bgr[index + 7], r2 = bgr[index + 8];
            const unsigned char b3 = bgr[index + 9], g3 = bgr[index + 10], r3 = bgr[index + 11];
            const unsigned char b4 = bgr[index + 12], g4 = bgr[index + 13], r4 = bgr[index + 14];
            const unsigned char b5 = bgr[index + 15], g5 = bgr[index + 16], r5 = bgr[index + 17];
            const unsigned char b6 = bgr[index + 18], g6 = bgr[index + 19], r6 = bgr[index + 20];
            const unsigned char b7 = bgr[index + 21], g7 = bgr[index + 22], r7 = bgr[index + 23];

            *(ptrY++) =  (unsigned char)((66 * r0 + 129 * g0 + 25 * b0 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r1 + 129 * g1 + 25 * b1 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r2 + 129 * g2 + 25 * b2 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r3 + 129 * g3 + 25 * b3 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r4 + 129 * g4 + 25 * b4 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r5 + 129 * g5 + 25 * b5 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r6 + 129 * g6 + 25 * b6 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r7 + 129 * g7 + 25 * b7 + 128) >> 8) + 16;

#define CLIP(x) ((unsigned char)std::min(255, std::max(0, static_cast<int>(x))))
            if (j % 2 == 0) {
                const unsigned char u0 = (unsigned char)((-38 * r0 - 74 * g0 + 112 * b0 + 128) >> 8) + 128;
                const unsigned char v0 = (unsigned char)((112 * r0 - 94 * g0 - 18 * b0 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u0);
                *(ptrUV++) = CLIP(v0);

                const unsigned char u2 = (unsigned char)((-38 * r2 - 74 * g2 + 112 * b2 + 128) >> 8) + 128;
                const unsigned char v2 = (unsigned char)((112 * r2 - 94 * g2 - 18 * b2 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u2);
                *(ptrUV++) = CLIP(v2);

                const unsigned char u4 = (unsigned char)((-38 * r4 - 74 * g4 + 112 * b4 + 128) >> 8) + 128;
                const unsigned char v4 = (unsigned char)((112 * r4 - 94 * g4 - 18 * b4 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u4);
                *(ptrUV++) = CLIP(v4);

                const unsigned char u6 = (unsigned char)((-38 * r6 - 74 * g6 + 112 * b6 + 128) >> 8) + 128;
                const unsigned char v6 = (unsigned char)((112 * r6 - 94 * g6 - 18 * b6 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u6);
                *(ptrUV++) = CLIP(v6);
            }
#undef CLIP
        }

        i = w_step << 3;
        unsigned char y, u, v, r, g, b;
        for (; i < w; i++) {
            const int index = base + i * 3;
            b = bgr[index];
            g = bgr[index + 1];
            r = bgr[index + 2];

            *(ptrY++) =  (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;

#define CLIP(x) ((unsigned char)std::min(255, std::max(0, static_cast<int>(x))))
            if (j % 2 == 0 and i % 2 == 0) {
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u);
                *(ptrUV++) = CLIP(v);
            }
#undef CLIP
        }
    }
}


void BGR2NV12v4(const unsigned char* bgr, const int w, const int h, unsigned char* nv12, const int out_w, const int out_h) {
    assert(out_w % 2 == 0);
    assert(out_w >= w and out_h >= h);
    const int AREA = out_w * out_h;
    unsigned char* base_uv = nv12 + AREA;
    // black YUV
    memset(nv12, 0, AREA);
    memset(nv12 + AREA, 128, AREA >> 1);

#ifdef __ARM_NEON
    int8x8_t _66 = vdup_n_s8(66);
    int8x8_t _129= vdup_n_s8(129);
    int8x8_t _128= vdup_n_s8(128);
    int8x8_t _25 = vdup_n_s8(25);
    int32x4_t _4224 = vdup_n_s32(4224);
    int16x4_t _0 = vdup_n_s16(0);

    int32x4_t _65536 = vdup_n_s32(65536);
    int32x4_t _130560 = vdup_n_s32(130560);
    int16x4_t _n38 = vdup_n_s16(-38);
    int16x4_t _n74 = vdup_n_s16(-74);
    int16x4_t _112 = vdup_n_s16(112);
    int16x4_t _n94 = vdup_n_s16(-94);
    int16x4_t _n18 = vdup_n_s16(-18);
#endif

    for (int j = 0; j < h; j++) {
        const int base = w * j * 3;
        unsigned char* ptrY = nv12 + out_w * j;
        unsigned char* ptrUV  = base_uv + out_w / 2 * j;

        const int w_step = w >> 3;
        int i = 0;
        for (; i < w_step; i++) {
            const int index = base + i * 24;
#ifdef __ARM_NEON
// to test
            int8x8x3_t _bgr = vld3_s8(bgr + index);
            int8x8_t _b = bgr[0];
            int8x8_t _g = bgr[1];
            int8x8_t _r = bgr[2];
            int32x4_t sum = vdupq_n_s32(0);

            int16x8_t _66r = vmull_s8(_66, _r);
            int16x8_t _129g = vmull_s8(_129, _g);
            int16x8_t _25b = vmull_s8(_128, _25);

            int32x4_t _sum = vaddl_s16(vget_low(_66r), vget_low(_129g));
            int32x4_t _sum1 = vaddl_s16(vget_low(_25b), _0);
            _sum = vaddq_s32(_sum, _sum1);
            _sum = vaddq_s32(_sum, _4224);
            uint16x4_t _res1 = vshrn_n_u32(vreinterpretq_u32_s32((_sum)), 8);

            _sum = vaddl_s16(vget_high(_66r), vget_high(_129g));
            _sum1 = vaddl_s16(vget_high(_25b), _0);
            _sum = vaddq_s32(_sum, _sum1);
            _sum = vaddq_s32(_sum, _4224);
            uint16x4_t _res2 = vshrn_n_u32(vreinterpretq_u32_s32((_sum)), 8);

            utin16x8_t _res = vcombine_u16(_res1, _res2);
            vst1_u8(ptrY, vshrn_n_u16(_res, 0));
            ptrY += 8;

            if (0 == j % 2) {
                int8x8_t _r = vtrn_s8(_r, _r)[0];
                int16x4_t _2r = vpaddl_s8(_r);
                int8x8_t _g = vtrn_s8(_g, _g)[0];
                int16x4_t _2g = vpaddl_s8(_g);
                int8x8_t _b = vtrn_s8(_b, _b)[0];
                int16x4_t _2b = vpaddl_s8(_b);

                int32x4_t _n38r = vmull_n_s16(_n38, _2r);
                int32x4_t _n74g = vmull_n_s16(_n74, _2g);
                sum = vaddq_s32(_n38r, _n74g);
                int32x4_t _112b = vmull_n_s16(_112, _2b);
                sum = vaddq_s32(sum, _112b);
                sum = vaddq_s32(sum, _65536);
                sum = vminq_s32(vmaxq_s32(sum, _0), _130560);
                uint16x4_t _u16 = vshrn_n_u32(vreinterpretq_u32_s32(sum), 9);
                uint8x8_t _u8 = vshrn_n_u32(vcombine_u16(_u16, _u16));

                int32x4_t _112r = vmull_n_s16(_112, _2r);
                int32x4_t _n94g = vmull_n_s16(_n44, _2g);
                sum = vaddq_s32(_112r, _n94g);
                int32x4_t _n18b = vmull_n_s16(_n18, _2b);
                sum = vaddq_s32(sum, _n18b);
                sum = vaddq_s32(sum, _65536);
                sum = vminq_s32(vmaxq_s32(sum, _0), _130560);
                uint16x4_t _v6 = vshrn_n_u32(vreinterpretq_u32_s32(sum), 9);
                uint8x8_t _v8 = vshrn_n_u32(vcombine_u16(_u16, _u16));

                vst2_u8(ptrUV, _u8, _v8);
                ptrUV += 8;
            }
#else
            const unsigned char b0 = bgr[index], g0 = bgr[index + 1], r0 = bgr[index + 2];
            const unsigned char b1 = bgr[index + 3], g1 = bgr[index + 4], r1 = bgr[index + 5];
            const unsigned char b2 = bgr[index + 6], g2 = bgr[index + 7], r2 = bgr[index + 8];
            const unsigned char b3 = bgr[index + 9], g3 = bgr[index + 10], r3 = bgr[index + 11];
            const unsigned char b4 = bgr[index + 12], g4 = bgr[index + 13], r4 = bgr[index + 14];
            const unsigned char b5 = bgr[index + 15], g5 = bgr[index + 16], r5 = bgr[index + 17];
            const unsigned char b6 = bgr[index + 18], g6 = bgr[index + 19], r6 = bgr[index + 20];
            const unsigned char b7 = bgr[index + 21], g7 = bgr[index + 22], r7 = bgr[index + 23];

            *(ptrY++) =  (unsigned char)((66 * r0 + 129 * g0 + 25 * b0 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r1 + 129 * g1 + 25 * b1 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r2 + 129 * g2 + 25 * b2 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r3 + 129 * g3 + 25 * b3 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r4 + 129 * g4 + 25 * b4 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r5 + 129 * g5 + 25 * b5 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r6 + 129 * g6 + 25 * b6 + 128) >> 8) + 16;
            *(ptrY++) =  (unsigned char)((66 * r7 + 129 * g7 + 25 * b7 + 128) >> 8) + 16;

#define CLIP(x) ((unsigned char)std::min(255, std::max(0, static_cast<int>(x))))
            if (j % 2 == 0) {
                const unsigned char u0 = (unsigned char)((-38 * r0 - 74 * g0 + 112 * b0 + 128) >> 8) + 128;
                const unsigned char v0 = (unsigned char)((112 * r0 - 94 * g0 - 18 * b0 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u0);
                *(ptrUV++) = CLIP(v0);

                const unsigned char u2 = (unsigned char)((-38 * r2 - 74 * g2 + 112 * b2 + 128) >> 8) + 128;
                const unsigned char v2 = (unsigned char)((112 * r2 - 94 * g2 - 18 * b2 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u2);
                *(ptrUV++) = CLIP(v2);

                const unsigned char u4 = (unsigned char)((-38 * r4 - 74 * g4 + 112 * b4 + 128) >> 8) + 128;
                const unsigned char v4 = (unsigned char)((112 * r4 - 94 * g4 - 18 * b4 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u4);
                *(ptrUV++) = CLIP(v4);

                const unsigned char u6 = (unsigned char)((-38 * r6 - 74 * g6 + 112 * b6 + 128) >> 8) + 128;
                const unsigned char v6 = (unsigned char)((112 * r6 - 94 * g6 - 18 * b6 + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u6);
                *(ptrUV++) = CLIP(v6);
            }
#undef CLIP
#endif
        }

        i = w_step << 3;
        unsigned char y, u, v, r, g, b;
        for (; i < w; i++) {
            const int index = base + i * 3;
            b = bgr[index];
            g = bgr[index + 1];
            r = bgr[index + 2];

            *(ptrY++) =  (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;

#define CLIP(x) ((unsigned char)std::min(255, std::max(0, static_cast<int>(x))))
            if (j % 2 == 0 and i % 2 == 0) {
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrUV++) = CLIP(u);
                *(ptrUV++) = CLIP(v);
            }
#undef CLIP
        }
    }
}


#define TEST(funcname) \
{\
    timeval begin;\
    gettimeofday(&begin, NULL);\
    const int REPEAT = 1000;\
    for (size_t  i = 0; i < REPEAT; ++i) {\
        funcname(m.data, m.cols, m.rows, yuv.data, out_w, out_h);\
    }\
    timeval end;\
    gettimeofday(&end, NULL);\
    double timecost = (end.tv_sec - begin.tv_sec) * 1000.0 + (end.tv_usec - begin.tv_usec) / 1000.0;\
    std::cout   << "avg timecost " << (timecost/REPEAT) << " ms" << std::endl; \
}

int main() {
    cv::Mat m1 = cv::imread("1080p.png");
    cv::Mat m;
    cv::resize(m1, m, cv::Size(1281, 720));
    if (m.empty()) {
        fprintf(stderr, "m is empty\n");
    }

    const  int out_w = 1920, out_h = 1080;
    cv::Mat yuv(out_h * 3 / 2, out_w, CV_8UC1);

    TEST(BGR2NV12v2)

    TEST(BGR2NV12v3)

    // timeval begin;
    // gettimeofday(&begin, NULL);
    // const int REPEAT = 1000;
    // for (size_t  i = 0; i < REPEAT; ++i) {
    //     BGR2NV12v1(m.data, m.cols, m.rows, yuv.data, out_w, out_h);
    // }
    // timeval end;
    // gettimeofday(&end, NULL);
    // double timecost = (end.tv_sec - begin.tv_sec) * 1000.0 + (end.tv_usec - begin.tv_usec) / 1000.0;
    // std::cout << "avg timecost " << (timecost/REPEAT) << " ms" << std::endl;

    // cv::cvtColor(m, yuv, cv::COLOR_BGR2YUV_I420);
    // FILE *fp_yuv = fopen("gt.yuv", "wb+");
    // fwrite(yuv.data, 1, yuv.cols * yuv.rows, fp_yuv);
    // fclose(fp_yuv);

    cv::Mat out;
    cv::cvtColor(yuv, out, cv::COLOR_YUV2BGR_NV12);
    cv::imwrite("dt.png", out);
}
