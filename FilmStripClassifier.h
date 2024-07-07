#ifndef FILMSTRIPCLASSIFIER_H
#define FILMSTRIPCLASSIFIER_H

#include "stable.h"

struct FilmStripClassifier {
    enum {
        NOT_PRESSED,
        NORMAL,
        OVER_ONE_THIRD,
        NOT_PRESSED_WELL
    };
    FilmStripClassifier() noexcept;
    int                              excute() noexcept;
    static void                      train() noexcept;
    static const char *              getEnumStr( const int iEnum ) noexcept;
    static std::pair< int, cv::Mat > getMeanHeight( cv::Mat &gray_image, cv::Mat &bgr_image, const AlgorithmParams::ParamsTupleType &rect_tuple ) noexcept;
    static int                       checkRect( cv::Mat &gray_image, cv::Mat &bgr_image, const AlgorithmParams::ParamsTupleType &rect_tuple ) noexcept;
    static int                       checkRound( const cv::Rect &rect, cv::Mat &gray_image, cv::Mat &bgr_image ) noexcept;

private:
    static void imageProcess( cv::Mat &image, int threshold ) noexcept;

    cv::Mat &image, &image_BGR;
};

#endif// FILMSTRIPCLASSIFIER_H
