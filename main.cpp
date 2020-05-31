#include <iostream>
#include <opencv/cv.hpp>
#include "mat.h"
#include <cmath>

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

int main() {
    cv::Mat m1 = cv::imread("1080p.png");
    cv::Mat m;
    cv::resize(m1, m, cv::Size(1998, 1000));
    if (m.empty()) {
        fprintf(stderr, "m is empty\n");
    }
    cv::Mat yuv(m.rows * 3 / 2, m.cols, CV_8UC1);
    BGR2YUV420P(m.data, m.cols, m.rows, yuv.data);
    // cv::cvtColor(m, yuv, cv::COLOR_BGR2YUV_I420);
    FILE *fp_yuv = fopen("gt.yuv", "wb+");
    fwrite(yuv.data, 1, yuv.cols * yuv.rows, fp_yuv);
    fclose(fp_yuv);

    cv::Mat out;
    cv::cvtColor(yuv, out, cv::COLOR_YUV2BGR_NV21);
    cv::imwrite("gt.png", out);


    cv::Mat out2(out.rows / 2, out.cols / 2, CV_8UC3);
    yuv420sp2rgb_half(yuv.data, m.cols, m.rows, out2.data);
    cv::Mat bgr;
    cv::cvtColor(out2, bgr, cv::COLOR_BGR2RGB);
    cv::imwrite("dt.png", bgr);
}
