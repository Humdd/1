#include "Cv2Qt.h"
#include "matchit.h"
#include <QTemporaryDir>

QImage Cv2Qt::Mat2QImage( const cv::Mat &image ) {
    Q_ASSERT( !image.empty() );
    using namespace matchit::impl;

    //match返回一个MatchHelper函数对象，所以采用match()()格式
    QImage &&dst = match( image.type() )(
        //"|"操作符重载，将CV_8UC4作为右参数传递给pattern
        //在match中CV_8UC4与image.type()相比较

        pattern | CV_8UC4 = [ &image ] { return QImage( image.data, image.cols, image.rows, static_cast< int >( image.step ), QImage::Format_ARGB32 ); },
        pattern | CV_8UC3 = [ &image ] { return QImage( image.data, image.cols, image.rows, static_cast< int >( image.step ), QImage::Format_BGR888 ); },
        pattern | CV_8UC1 = [ &image ] { return QImage( image.data, image.cols, image.rows, static_cast< int >( image.step ), QImage::Format_Grayscale8 ); },
        pattern | _       = [ &image ] {
        QTemporaryDir dir; Q_ASSERT( dir.isValid() == true );
        const auto &temporary_image_path = QString( "%1/temp.bmp" ).arg( dir.path() );
        const bool ret = cv::imwrite( temporary_image_path.toStdString(), image ); Q_ASSERT( ret );
        return QImage( temporary_image_path ); } );
    Q_ASSERT( !dst.isNull() );
    return dst;
}

cv::Mat Cv2Qt::QPixmap2Mat( const QPixmap &pixmap, const int mode ) {
    Q_ASSERT( pixmap.isNull() == false );
    const auto &tmp_path = QString( "%1/temp.bmp" ).arg( QTemporaryDir().path() );
    pixmap.save( tmp_path );
    cv::Mat image = cv::imread( tmp_path.toStdString(), mode );
    Q_ASSERT( image.empty() == false );
    return image;
}

QPixmap Cv2Qt::Mat2Pixmap( const cv::Mat &image ) {
    Q_ASSERT( !image.empty() );
    return QPixmap::fromImage( Mat2QImage( image ) );
}

//QImage::Format_Indexed8 QImage::Format_Grayscale8
cv::Mat Cv2Qt::QImage2Mat( const QImage &image, const int mode ) {
    Q_ASSERT( !image.isNull() );
    using namespace matchit::impl;
    cv::Mat &&dst = match( image.format() )(
        pattern | or_( QImage::Format_ARGB32, QImage::Format_RGB32, QImage::Format_ARGB32_Premultiplied ) = [ &image ] { return cv::Mat( image.height(), image.width(), CV_8UC4, ( void * )image.bits(), image.bytesPerLine() ); },
        pattern | QImage::Format_RGB888                                                                   = [ &image ] {
        auto &&mat = cv::Mat( image.height(), image.width(), CV_8UC3, ( void * )image.constBits(), image.bytesPerLine() ); cv::cvtColor( mat, mat, cv::COLOR_BGR2RGB ); return mat; },
        pattern | QImage::Format_Grayscale8                                                               = [ &image ] { return cv::Mat( image.height(), image.width(), CV_8UC1, ( void * )image.bits(), image.bytesPerLine() ); },
        pattern | _                                                                                       = [ &image, mode ] {
        const auto &tmp_path = QString( "%1/temp.bmp" ).arg( QTemporaryDir().path() );        image.save( tmp_path );
        return cv::imread( tmp_path.toStdString(), mode ); } );
    Q_ASSERT( !dst.empty() );
    return dst;
}

std::string Cv2Qt::validImagePath( const QString &image_path ) noexcept {
    Q_ASSERT( !image_path.isEmpty() );
    Q_ASSERT( !image_path.contains( "\\" ) );
    return image_path.toLocal8Bit().toStdString();
}
