#pragma once

#include "SupportClass/MyTypeTraits.h"
#include "core/types.hpp"

/*！
 *  这是一个表示一条直线表达式的类（ 直线表达式 ： Ax + By + C = 0 ），具有以下功能：
 *  （1）读入opencv的点类来初始化类，可以得到穿过两个点的直线方程参数（A、B、C）
 *  （2）传入x、y值，求得在直线上的y、x值
 *  （3）求两条直线的交点坐标
 *  （4）求点到直线的距离
 */
struct LineData {
    LineData( const double A , const double B , const double C ) noexcept
            : A( A )
              , B( B )
              , C( C ) {}

    template< typename T , typename POINT_TYPE = remove_cvf_t< T > , TYPE_CONSTRAINT( , (
            std::is_same_v< POINT_TYPE , cv::Point > || std::is_same_v< POINT_TYPE , cv::Point2d > ||
            std::is_same_v< POINT_TYPE , cv::Point2l > || std::is_same_v< POINT_TYPE , cv::Point2f > ) ) >
    LineData( const T &point1 , const T &point2 ) noexcept
            : A( point2.y - point1.y ) , B( point1.x - point2.x ) , C( point2.x * point1.y - point1.x * point2.y ) {}

    double getY( const double x ) const noexcept {
        Q_ASSERT( B != 0 );
        return ( -C - A * x ) / B;
    }

    double getX( const double y ) const noexcept {
        Q_ASSERT( A != 0 );
        return ( -C - B * y ) / A;
    }

    template< typename T , typename POINT_TYPE = remove_cvf_t< T > , TYPE_CONSTRAINT( , (
            std::is_same_v< POINT_TYPE , cv::Point > || std::is_same_v< POINT_TYPE , cv::Point2d > ||
            std::is_same_v< POINT_TYPE , cv::Point2l > || std::is_same_v< POINT_TYPE , cv::Point2f > ) ) >
    double getDistance( const T &point ) const noexcept {
        Q_ASSERT( !qFuzzyCompare( A * A + B * B , 0.0 ) );
        return fabs( A * point.x + B * point.y + C ) / sqrt( A * A + B * B );
    }

    cv::Point2d getIntersection( const LineData &other ) const noexcept {
        return getIntersection( A , B , C , other.A , other.B , other.C );
    }

    static cv::Point2d
    getIntersection( const double A1 , const double B1 , const double C1 , const double A2 , const double B2
                     , const double C2 ) noexcept {
        const double m = A1 * B2 - A2 * B1;
        Q_ASSERT( m != 0 );
        return {( C2 * B1 - C1 * B2 ) / m , ( C1 * A2 - C2 * A1 ) / m};
    }

    double A , B , C;
};
