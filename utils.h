#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <numeric>
#include <cmath>
#include <QDebug>

#include "opencv2/opencv.hpp"

class Utils
{
public:
    Utils();

    std::vector<std::vector<cv::Mat>> partitionImage(cv::Mat &src, int rows, int cols);

    static void callback(int value, void*);
    // 滑动条二值化
    void binarizationWithSlider(cv::Mat &src);

    cv::Scalar getAvg(cv::Mat &src);
    //统计图片白点数量
    int getWhiteCnt(cv::Mat &src);
    int getBlackCnt(cv::Mat &src);
    double getMinVal(cv::Mat &src);
    double getMaxVal(cv::Mat &src);
    double getMean(cv::Mat &src);
    double getStdDev(cv::Mat &src);
    double getRatio(int x, int y);
    std::vector<int> getBlockWhiteCnts(cv::Mat &src);
    std::vector<int> getBlockWhiteCntsTest(cv::Mat &src);
    std::vector<double> getRatioWithinVector(std::vector<int> &v);
    std::vector<double> getRatioBetweenVector(std::vector<int> &v1, std::vector<int> &v2);
    void printStatistics(cv::Mat &src);
    void printVector(std::vector<int> &v);
    void printVector(std::vector<double> &v);
    //获取缺陷类型
    int getType(cv::Mat &src);
    int getTypeTest(cv::Mat &src);

    cv::Rect getMatchResultROI(cv::Mat &img, const cv::Mat &templ);
    cv::Mat calcHist(cv::Mat &img);
    int getDiffThresh(cv::Mat &gau_grayDiff);
    bool inspectLogo(cv::Mat &chip);
    bool inspectLogo(cv::Mat &img, cv::Mat &templ, cv::Mat &templ_bin, cv::Mat &mask, int thresh=200, int wnd_sz=5);
    bool ocv(cv::Mat &img, cv::Mat &templ, cv::Mat &templ_bin, cv::Mat &mask);

    static cv::Mat img1, img2;
//    cv::Mat templ; //模板
//    cv::Mat templ_bin; //模板二值图
//    cv::Mat logoMask;
};

#endif // UTILS_H
