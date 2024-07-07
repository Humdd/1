#ifndef OPENCVHELP_H
#define OPENCVHELP_H

#include "core/mat.hpp"
#include "core/types.hpp"
#include "SupportClass/MyTypeTraits.h"
#include <QVector>

/*!
 *  这是一个简单封装了一些opencv调用的命名空间（类），主要接口功能包括：
 *  （1）与图像旋转/缩放有关：获取图像的旋转角度、旋转缩放图像
 *  （2）在图像上作画：画矩形、写字
 *  （3）一个二值化算法：Bernsen二值化
 *  （4）一些点、矩形的简单辅助函数：得到一个矩形的中心点坐标、求一个点旋转一定角度后的位置
 */
struct OpenCVHelp {
    static double getImageRotateAngle( const cv::Mat &image );

    static cv::Mat resizeImage( const cv::Mat &image, double scaling_multiplier );

    static cv::Mat
        rotateArbitrarilyAngle( const cv::Mat &image, double angle, double scale = 1, const cv::Scalar &scalar = { 255, 255, 255 } );

    static cv::Mat Bernsen( const cv::Mat &src, int blockSize, int Th, int S );

    template< typename RectType, std::enable_if_t< is_cv_rect_v< RectType >, int > = 0 >
    inline static void drawRect( cv::Mat &image, const RectType &rect, const cv::Scalar &scalar = { 0, 255, 0 }, const int thickness = draw_thickness ) noexcept {
        Q_ASSERT( !rect.empty() );
        Q_ASSERT( !image.empty() );
        const cv::Point points[ 4 ] = {
            cv::Point( rect.x, rect.y ), cv::Point( rect.x + rect.width, rect.y ), cv::Point( rect.x + rect.width, rect.y + rect.height ), cv::Point( rect.x, rect.y + rect.height )
        };
        int              npts[] = { 4 };
        const cv::Point *pts[]  = { points };
        cv::polylines( image, pts, npts, 1, true, scalar, thickness );
    }

    inline static void
        drawText( cv::Mat &image, const cv::Point &pos, const std::string &text, const cv::Scalar &scalar = { 0, 255, 0 }, const int thickness = draw_thickness + 1, const double fontScale = 3.5 ) noexcept {
        Q_ASSERT( !text.empty() );
        Q_ASSERT( !image.empty() );
        Q_ASSERT( pos.x >= 0 && pos.x < image.cols );
        Q_ASSERT( pos.y >= 0 && pos.y < image.rows );
        putText( image, text, pos, cv::FONT_HERSHEY_PLAIN, fontScale, scalar, thickness );
    }

    static cv::Point
        getRotatedPointPos( const cv::Point &point1, const cv::Point &center_point, const double rotated_angle );

    static cv::Point2d getCenterOfRect( const cv::Rect &rect );

private:
    const static int draw_thickness = 3;
};

#endif// OPENCVHELP_H
