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
    cv::resize(m1, m, cv::Size(1920, 1080));
    if (m.empty()) {
        fprintf(stderr, "m is empty\n");
    }

    const  int out_w = 1920, out_h = 1080;
    cv::Mat yuv(out_h * 3 / 2, out_w, CV_8UC1);

    TEST(BGR2NV12v1)

    TEST(BGR2NV12v2)

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
