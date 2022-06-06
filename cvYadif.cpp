#include "cvYadif.h"

void ReYadif1Channel(
    int mode,
    uint8_t *dst,
    const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int step1,
    int w, int h,
    int parity, int tff);

namespace cv {

    Yadif::Yadif(const cv::Mat &first)
    {
        buf[0].resize(3);
        buf[1].resize(3);
        buf[2].resize(3);
        buf[3].resize(3);

        cv::split(first, buf[0]);
        cv::split(first, buf[1]);
        cv::split(first, buf[2]);
        cv::split(first, buf[3]);
        inputPosition = 0;
        frame = 1;
    }

    void Yadif::deinterlace(const cv::Mat &input, cv::Mat &output)
    {
        cv::split(input, buf[inputPosition]);
        inputPosition = (inputPosition + 1) & 0x03;

        const int prei = (inputPosition - 3) & 0x03;
        const int curi = (inputPosition - 2) & 0x03;
        const int nexi = (inputPosition - 1) & 0x03;
        const int dsti = (inputPosition - 0) & 0x03;

        const std::vector<cv::Mat> & cur = buf[curi];
        const std::vector<cv::Mat> & pre = buf[prei];
        const std::vector<cv::Mat> & nex = buf[nexi];
        const std::vector<cv::Mat> & dst = buf[dsti];

        ReYadif1Channel(0, dst[0].data, pre[0].data, cur[0].data, nex[0].data, pre[0].step1(), dst[0].cols, dst[0].rows, 0, 0);
        ReYadif1Channel(0, dst[1].data, pre[1].data, cur[1].data, nex[1].data, pre[1].step1(), dst[1].cols, dst[1].rows, 0, 0);
        ReYadif1Channel(0, dst[2].data, pre[2].data, cur[2].data, nex[2].data, pre[2].step1(), dst[2].cols, dst[2].rows, 0, 0);

        cv::merge(dst, output);

        frame++;
    }

};