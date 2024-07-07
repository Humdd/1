#ifndef ICCLASSIFIER_H
#define ICCLASSIFIER_H

#include "stable.h"

struct ICClassifier {
    enum ChipStatus{
        EMPTY,
        NORMAL,
        ROTATED,
        FLIPPED,
        LOGO_ABNORMAL
    };
    ICClassifier() noexcept;
    static void                      binaryThreshold( cv::Mat &gray_image, cv::Mat &rgb_image, const cv::Rect &rect, const int threshold ) noexcept;
    int                              execute1() noexcept;
    int                              execute() noexcept;
    static void                      train() noexcept;
    static const char *              getEnumStr( const int iEnum ) noexcept;
    static std::pair< int, cv::Mat > getMeanHeight( cv::Mat &gray_image, cv::Mat &bgr_image, const ICAlgorithmParams::ParamsTupleType &rect_tuple ) noexcept;
    static int                       checkRect( cv::Mat &gray_image, cv::Mat &bgr_image, const ICAlgorithmParams::ParamsTupleType &rect_tuple ) noexcept;
    static int                       checkRound( const cv::Rect &rect, cv::Mat &gray_image, cv::Mat &bgr_image ) noexcept;
    static int                       detectCircle( const cv::Rect &rect, cv::Mat &image_gray, cv::Mat &image_bgr, int minDist, int cannyThresh, int precision, int minRadius, int maxRadius, ChipStatus chipStatus) noexcept;
    static int                       detectCircle( const cv::Mat &src, int minDist, int cannyThresh, int precision, int minRadius, int maxRadius) noexcept;

private:
    static void imageProcess( cv::Mat &image, int threshold ) noexcept;

    cv::Mat &image, image_BGR; //image是原图
};

#endif// ICCLASSIFIER_H
