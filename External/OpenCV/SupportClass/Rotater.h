#ifndef ROTATER_H
#define ROTATER_H
#include "core/mat.hpp"
#include "core/matx.hpp"
/*！
 * Rotater类为辅助图像旋转的一个私有类，主要功能包括：
 * （1）求得图像的整体旋转角度
 */
struct Rotater {
    static double getImageRotateAngle( const cv::Mat &image );

private:
    //以下都为图像角度矫正算法的辅助函数
    static double angleOfLines( const cv::Vec4i &line1, const cv::Vec4i &line2 );
    static bool   getMinMidX( const cv::Vec4i &line1, const cv::Vec4i &line2 );
    static bool   getMaxMidX( const cv::Vec4i &line1, const cv::Vec4i &line2 );
    static bool   getMinMidY( const cv::Vec4i &line1, const cv::Vec4i &line2 );
    static bool   getMaxMidY( const cv::Vec4i &line1, const cv::Vec4i &line2 );
    static double angleForCorrect( const cv::Vec4i &line );
};

#endif// ROTATER_H
