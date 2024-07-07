#include "OpenCVHelp.h"
#include "SupportClass/Rotater.h"

cv::Mat OpenCVHelp::resizeImage( const cv::Mat &image , const double scaling_multiplier ) {
    if( qFuzzyCompare( scaling_multiplier , 1.0 ) ) return image;
    cv::Mat dst = ( cv::Mat::zeros( image.rows * scaling_multiplier , image.cols * scaling_multiplier , CV_8UC3 ) );
    resize( image , dst , dst.size() , 0 , 0 , cv::INTER_LINEAR );//INTER_LINEAR INTER_CUBIC INTER_NEAREST
    return dst;
}

cv::Mat
OpenCVHelp::rotateArbitrarilyAngle( const cv::Mat &image , double angle , double scale , const cv::Scalar &scalar ) {
    if( abs( angle ) <= 0.0001 )
        if( ( scale - 1 ) < 0.0001 ) return image;
        else
            return resizeImage( image , scale );
    else;
    double angle2 = angle * CV_PI / 180.0;
    int width = image.cols;
    int height = image.rows;
    double alpha = cos( angle2 );
    double beta = sin( angle2 );
    cv::Point2d center( double(( double )width / 2 ), double( ( double )height / 2 ) );
    //计算二维旋转的仿射变换矩阵
    cv::Mat M( getRotationMatrix2D( center , angle , scale ) );
    // 给计算得到的旋转矩阵添加平移
    int new_width = ( int )( width * fabs( alpha ) + height * fabs( beta ) ) * scale;
    int new_height = ( int )( width * fabs( beta ) + height * fabs( alpha ) ) * scale;
    M.at< double >( 0 , 2 ) += ( int )( ( new_width - width ) / 2 );
    M.at< double >( 1 , 2 ) += ( int )( ( new_height - height ) / 2 );
    // rotate
    cv::Mat dst;
    cv::warpAffine( image , dst , M , cv::Size( new_width , new_height ) , cv::INTER_LINEAR
                    ,// INTER_CUBIC INTER_LINEAR INTER_NEAREST
            cv::BORDER_CONSTANT , scalar );
    return dst;
}

cv::Mat OpenCVHelp::Bernsen( const cv::Mat &src , int blockSize , int Th , int S ) {
    cv::Mat padded( src );
    cv::Mat dst( src );
    //对原图的边缘进行扩展
    cv::copyMakeBorder( src , padded , blockSize , blockSize , blockSize , blockSize , cv::BORDER_CONSTANT
                        , cv::Scalar::all( 255 ) );
    int row = padded.rows;
    int col = padded.cols;
    for( int i = blockSize ; i < row - blockSize ; i++ ) {
        const uchar *p = padded.ptr< const uchar >( i );
        uchar *q = dst.ptr< uchar >( i - blockSize );
        for( int j = blockSize ; j < col - blockSize ; j++ ) {
            //求(i,j)像素点邻域内的最大值和最小值
            int max = 0 , min = 256;
            int T = 0;
            for( int k = i - blockSize ; k < i + blockSize ; k++ ) {
                const uchar *ptr = padded.ptr< const uchar >( k );
                for( int l = j - blockSize ; l < j + blockSize ; l++ ) {
                    if( ptr[ l ] > max ) {
                        max = ptr[ l ];
                    }
                    if( ptr[ l ] < min ) {
                        min = ptr[ l ];
                    }
                }
            }
            //求max和min的均值
            T = ( max + min ) >> 1;
            if( ( max - min ) > S ) {//如果max-min>S,那么阈值设为T
                q[ j - blockSize ] = p[ j ] > T ? 255 : 0;
            }
            else {//若T>Th,则当前像素点灰度值为255;否则，当前像素点灰度值为0.
                q[ j - blockSize ] = T > Th ? 255 : 0;
            }
        }
    }
    return dst;
}

cv::Point
OpenCVHelp::getRotatedPointPos( const cv::Point &point1 , const cv::Point &center_point , const double rotated_angle ) {
    const auto &angle = ( 360 - rotated_angle ) * CV_PI / 180;
    return cv::Point(
            ( point1.x - center_point.x ) * std::cos( angle ) - ( point1.y - center_point.y ) * std::sin( angle ) +
            center_point.x ,
            ( point1.x - center_point.x ) * std::sin( angle ) + ( point1.y - center_point.y ) * std::cos( angle ) +
            center_point.y );
}

cv::Point2d OpenCVHelp::getCenterOfRect( const cv::Rect &rect ) {
    return {rect.x + rect.width / 2.0 , rect.y + rect.height / 2.0};
}

double OpenCVHelp::getImageRotateAngle( const cv::Mat &image ) {
    return Rotater::getImageRotateAngle( image );
}
