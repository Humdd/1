#ifndef CV2QT_H
#define CV2QT_H
#include "SupportClass/MyTypeTraits.h"
#include "core/mat.hpp"
#include "imgcodecs.hpp"
#include <QPixmap>
//---------------------------------------------------------------------------------
/*!
 *@brief   把QT与OpenCV里面对应数据类型进行互转的类，目前实现的功能如下：
 *         （1）把图片格式互转
 *         （2）把点类、矩形类互转
 */
//---------------------------------------------------------------------------------
struct Cv2Qt {
    static QImage      Mat2QImage( const cv::Mat &image );
    static cv::Mat     QPixmap2Mat( const QPixmap &pixmap, const int mode = cv::IMREAD_COLOR );
    static QPixmap     Mat2Pixmap( const cv::Mat &image );
    static cv::Mat     QImage2Mat( const QImage &image, const int mode = cv::IMREAD_COLOR );
    static std::string validImagePath( const QString &image_path ) noexcept;
    template< typename S, typename Elem = remove_cvf_t< decltype( std::declval< remove_cvf_t< S > >()[ std::declval< int >() ] ) >, TYPE_HAS_MEM( HasMem_rx, &Elem::rx ), TYPE_HAS_MEM( HasMem_ry, &Elem::ry ), TYPE_CONSTRAINT(, std::is_class_v< Elem > ) >
    inline static cv::Rect pointArray2Rect( const S &point_array ) noexcept { return cv::Rect( cv::Point( point_array[ 0 ].x(), point_array[ 0 ].y() ), cv::Point( point_array[ 2 ].x(), point_array[ 2 ].y() ) ); }
    template< typename S, typename Elem = remove_cvf_t< decltype( std::declval< S >()[ std::declval< int >() ] ) >, std::enable_if_t< is_cv_point_v< Elem >, int > = 0 >
    inline static cv::Rect pointArray2Rect( const S &point_array ) noexcept { return cv::Rect( cv::Point( point_array[ 0 ].x, point_array[ 0 ].y ), cv::Point( point_array[ 2 ].x, point_array[ 2 ].y ) ); }
    template< typename D, typename S, TYPE_HAS_MEM( HasMem_x_2, std::declval< S >().x() ), TYPE_HAS_MEM( HasMem_y_2, std::declval< S >().y() ) >
    inline static D convertRect( const S &src ) noexcept { return D( src.x(), src.y(), src.width(), src.height() ); }
    template< typename D, typename S, std::enable_if_t< is_cv_rect_v< S >, int > = 0 >
    inline static D convertRect( const S &src ) noexcept { return D( src.x, src.y, src.width, src.height ); }
    template< typename D, typename S, std::enable_if_t< is_cv_point_v< S >, int > = 0 >
    inline static D convertPoint( const S &src ) noexcept { return D( src.x, src.y ); }
    template< typename D, typename S, TYPE_HAS_MEM( HasMem_rx, &S::rx ), TYPE_HAS_MEM( HasMem_ry, std::declval< S >().ry() ) >
    inline static D convertPoint( const S &src ) noexcept { return D( src.x(), src.y() ); }
};

#define CNImagePath( STR ) QStringLiteral( STR ).toStdString()

#endif// CV2QT_H
