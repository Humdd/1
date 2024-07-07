#ifndef OPENCVSERIALIZATION_H
#define OPENCVSERIALIZATION_H

#include "MyTypeTraits.h"
#include "core/types.hpp"
#include <QDataStream>

/*！
 *  以下是一些序列化opencv中基础类型的接口（全是惰性的函数模板），支持的数据结构包括：
 *  （1）所有点类：cv::Point、cv::Point2d等等
 *  （2）所有矩形类：cv::Rect、cv::Rect2d等等
 */

template< typename T, std::enable_if_t< is_cv_point_v< T >, int > = 0 >
inline static QDataStream &operator<<( QDataStream &stream, const T &d ) noexcept {
    return stream << d.x << d.y;
}

template< typename T, std::enable_if_t< is_cv_point_v< T >, int > = 0 >
inline static QDataStream &operator>>( QDataStream &stream, T &d ) noexcept {
    return stream >> d.x >> d.y;
}

template< typename T, std::enable_if_t< is_cv_rect_v< T >, int > = 0 >
inline static QDataStream &operator<<( QDataStream &stream, const T &d ) noexcept {
    return stream << d.x << d.y << d.width << d.height;
}

template< typename T, std::enable_if_t< is_cv_rect_v< T >, int > = 0 >
inline static QDataStream &operator>>( QDataStream &stream, T &d ) noexcept {
    return stream >> d.x >> d.y >> d.width >> d.height;
}

#endif// OPENCVSERIALIZATION_H
