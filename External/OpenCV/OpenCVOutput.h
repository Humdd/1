#ifndef OPENCVOUTPUT_H
#define OPENCVOUTPUT_H

#include "SupportClass/MyTypeTraits.h"
#include "core/types.hpp"
#include <QDebug>
#include <type_traits>
/*！
 *  以下是一些用QDebug输出opencv中基础类型的接口（全是惰性的函数模板），支持的数据结构包括：
 *  （1）所有点类：cv::Point、cv::Point2d等等
 *  （2）所有矩形类：cv::Rect、cv::Rect2d等等
 *  （3）cv::Scalar类
 */

template< typename T, std::enable_if_t< is_cv_point_v< T >, int > = 0 >
inline static QDebug &operator<<( QDebug &debug, const T &point ) noexcept {
    return debug << "cv::Point(" << point.x << "," << point.y << ")";
}

template< typename T, std::enable_if_t< is_cv_rect_v< T >, int > = 0 >
inline static QDebug &operator<<( QDebug &debug, const T &rect ) noexcept {
    return debug << "cv::Rect(" << rect.x << "," << rect.y << "," << rect.width << "," << rect.height << ")";
}

template< typename T, std::enable_if_t< std::is_same_v< remove_cvf_t< T >, cv::Scalar >, int > = 0 >
inline static QDebug &operator<<( QDebug &debug, const T &scalar ) noexcept {
    return debug << "cv::Scalar(" << scalar[ 0 ] << "," << scalar[ 1 ] << "," << scalar[ 2 ] << "," << scalar[ 3 ]
                 << ")";
}

#endif// OPENCVOUTPUT_H
