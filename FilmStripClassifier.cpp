#include "FilmStripClassifier.h"
#include "AbstractGraphicsItem.h"
#include "LineData.h"
#include <execution>
#include "ConsoleFileLogger.h"

FilmStripClassifier::FilmStripClassifier() noexcept
    : image( params.excute_gray_image )
    , image_BGR( params.excute_bgr_image ) {
    Q_ASSERT( params.isParamsValid() );
}

int FilmStripClassifier::excute() noexcept {
    Q_ASSERT( params.isParamsValid() );
    NoteTimingStart;
    TimeRecorder timer;
    //    int          result = NORMAL;
    //std::atomic原子类型的对象是唯一没有数据竞争的c++对象;也就是说，如果一个线程写一个原子对象，而另一个线程从它读取，这种行为是定义良好的。
    std::atomic_int result{ NORMAL };

    //find_if函数找到容器中lambda规定的元素
    IGNORE_RESULT(
        std::find_if(
            std::execution::par_unseq, params.rect_tuple_array.begin(), params.rect_tuple_array.end(),
            [ this, &result ]( auto &&rect_tuple ) {
                const auto type = std::get< 3 >( rect_tuple );
                switch( type ) {
                //当type是AbstractGraphicsItem::RECT_UP和AbstractGraphicsItem::RECT_DOWN时执行相同的函数
                    case AbstractGraphicsItem::RECT_UP:
                    case AbstractGraphicsItem::RECT_DOWN: {
                        std::atomic_int rect_result{ checkRect( image, image_BGR, rect_tuple ) };
                        if( result.load() == NORMAL && rect_result.load() != NORMAL ) {
                            result.store( rect_result );
                        }
                        return result.load() != NORMAL;
                    } break;
                    case AbstractGraphicsItem::CIRCLE: {
                        return std::find_if(
                                   std::execution::par_unseq, params.rect_circle_array.begin(),
                                   params.rect_circle_array.end(), [ this, &result ]( auto &&circle ) {
                                   //检查圆圈部分是否合格
                                       std::atomic_int circle_result{ checkRound( *circle, image, image_BGR ) };
                                       if( result.load() == NORMAL && circle_result.load() != NORMAL ) {
                                           result.store( circle_result );
                                       }
                                       return result.load() != NORMAL;
                                   } ) != params.rect_circle_array.end();
                    } break;
                    default:
                        exit( 3 );
                        return true;
                        break;
                }
            } ) );
    timer.printTime( TIMER_RECORDER_OUTPUT( "excute" ) );
#ifdef _DEBUG
    //    if( !store_image_path.isEmpty() ) {
    //    const auto succeed = cv::imwrite( Cv2Qt::validImagePath( QString( store_image_path ).replace( ".bmp", "-result.bmp" ) ), image_BGR );
    //    Q_ASSERT( succeed );
    //    }
    //    cv::resize( image_BGR, image_BGR, { 1280, 720 } );
    //    cv::imshow( Cv2Qt::validImagePath( store_image_path ), image_BGR );
    //    cv::imshow( "image_BGR", image_BGR );
    //    cv::waitKey();
#endif
    params.algorithmFinished();
    printVar(result.load());
    printVar(getEnumStr(result.load()));
    return result.load();
}

//热合相机训练膜的边缘部分，测量白色选区的黑色矩形部分面积大小并且求取平均数
void FilmStripClassifier::train() noexcept {
    params.resetParams();
    TIMER_RECORD_FUNC_TIME;
    std::for_each(
        std::execution::par_unseq, params.rect_tuple_array.begin(), params.rect_tuple_array.end(),
        []( auto &&rect_tuple ) {
            auto &[ rect_ptr, _, mean_height, type ] = rect_tuple;
            switch( type ) {
                case AbstractGraphicsItem::RECT_UP:
                case AbstractGraphicsItem::RECT_DOWN: {
                    if( !rect_ptr ) return;
                    //求取矩形的平均高度
                    const auto white_point_count = FilmStripClassifier::getMeanHeight( params.excute_gray_image, params.excute_bgr_image, rect_tuple ).first;
                    mean_height                  = rect_ptr->height - white_point_count / double( rect_ptr->width );
                } break;
                case AbstractGraphicsItem::CIRCLE:
                //遍历容器内的元素并执行checkRound函数
                    std::for_each(
                        std::execution::par_unseq, params.rect_circle_array.begin(), params.rect_circle_array.end(),
                        []( auto &&circle_rect_ptr ) {
                            Q_ASSERT( circle_rect_ptr );
                            const auto &rect = *circle_rect_ptr;
                            Q_ASSERT( !rect.empty() );
                            FilmStripClassifier::checkRound( rect, params.excute_gray_image, params.excute_bgr_image );
                        } );
                    break;
            }
        } );
}

//求取高度平均值的函数
std::pair< int, cv::Mat > FilmStripClassifier::getMeanHeight(
    cv::Mat &gray_image, cv::Mat &bgr_image,
    const AlgorithmParams::ParamsTupleType &rect_tuple ) noexcept {
    //    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( gray_image.channels() == 1 );
    Q_ASSERT( bgr_image.channels() == 3 );
    Q_ASSERT( params.status == AlgorithmParams::SET_EXCUTE_IMG );
    const auto &[ rect_ptr, threshold, _, type ] = rect_tuple;
    const auto &rect                             = *rect_ptr;
    cv::Mat     roi                              = gray_image( rect ).clone();
    Q_ASSERT( !roi.empty() );
    imageProcess( roi, threshold );
    Q_ASSERT( !roi.empty() );
    cv::Mat roi_bgr = bgr_image( rect );
    cv::cvtColor( roi, roi_bgr, cv::COLOR_GRAY2RGB );
    const auto white_point = countNonZero( roi );
    OpenCVHelp::drawText( bgr_image, rect.tl() + cv::Point( 0, -100 ),
                          fmtStdStr( "%1", rect.height - white_point / double( rect.width ) ), { 0, 255, 0 } );
    return { white_point, roi };
}


//检测矩形（还没有图）
int FilmStripClassifier::checkRect( cv::Mat &gray_image, cv::Mat &bgr_image, const AlgorithmParams::ParamsTupleType &rect_tuple ) noexcept {
    Q_ASSERT( gray_image.channels() == 1 );
    Q_ASSERT( bgr_image.channels() == 3 );
    Q_ASSERT( !bgr_image.empty() );
    using namespace std;
    using namespace cv;
    TimeRecorder timer;
    int          result                                 = NORMAL;
    const auto &[ rect_ptr, threshold, mean_height, _ ] = rect_tuple;
    const auto &rect                                    = *rect_ptr;
    //    const int   max_threshold                           = rect.area() * params.max_black_point_ratio;
    std::string display_str;
    Scalar      scalar;
    const auto &[ roi_white_count, roi ] = getMeanHeight( gray_image, bgr_image, rect_tuple );
    const auto current_mean_height       = rect.height - roi_white_count / double( rect.width );
    if( roi_white_count >= 0.95 * rect.area() ) {
        //        printVar( roi_white_count );
        //        printVar( rect.area() );
        //        printVar( roi_white_count / ( rect.area() * 1.0 ) );
        //        printSeparate;
        result      = NOT_PRESSED;
        display_str = fmtStdStr( "Result:%1 nonZero:%2 rect.area():%3  mean_height:%4",
                                 getEnumStr( result ), roi_white_count,
                                 rect.area(), current_mean_height );
    }
    //    else if( const int black_point = rect.area() - roi_white_count;
    //             black_point > max_threshold ) {
    //        //        printVar( black_point );
    //        //        printVar( max_threshold );
    //        //        printSeparate;
    //        result      = NORMAL;
    //        display_str = fmtStdStr( "Result:%1 black_point:%2 white_count:%3  mean_height:%4",
    //                                 getEnumStr( result ), black_point,
    //                                 max_threshold, current_mean_height );
    //    }
    else {
        int       invalid_col_count       = 0;
        const int invalid_col_threshold   = AlgorithmParams::invalid_col_ratio * rect.width;
        int       invalid_point_threshold = AlgorithmParams::invalid_point_ratio * mean_height;
        for( int col = 0; col < roi.cols; ++col ) {
            //获取某一列
            auto &&col_mat = roi.colRange( col, col + 1 );
            Q_ASSERT( !col_mat.empty() );
            //统计该列黑点数量
            int black_point_count = roi.rows - cv::countNonZero( col_mat );
            //        printVar( black_point_count );
            if( black_point_count < invalid_point_threshold &&
                ++invalid_col_count >= invalid_col_threshold ) {
                result = NOT_PRESSED_WELL;
                break;
            }
        }
        //        printVar( invalid_col_count );
        //        printVar( invalid_col_threshold );
        //        printSeparate;
        display_str = fmtStdStr( "Result:%1 col_count:%2 col_threshold:%3 mean_height:%4",
                                 getEnumStr( result ), invalid_col_count, invalid_col_threshold, current_mean_height );
    }
    OpenCVHelp::drawText( bgr_image, rect.tl() + Point( 0, -50 ), display_str, { 0, 255, 0 } );
    //    timer.printTime( TIMER_RECORDER_OUTPUT( "checkRect" ) );
    return result;
}


//检测圆形（红色部分）和选区内直线之间的距离
int FilmStripClassifier::checkRound( const cv::Rect &rect, cv::Mat &gray_image, cv::Mat &bgr_image ) noexcept {
    using namespace std;
    using namespace cv;
    Q_ASSERT( !gray_image.empty() );
    Q_ASSERT( !bgr_image.empty() );
    Q_ASSERT( gray_image.channels() == 1 );
    Q_ASSERT( bgr_image.channels() == 3 );
    TimeRecorder timer;
    Mat          dst = gray_image( rect ) /*.clone()   - 150*/;
    Q_ASSERT( !dst.empty() );
    //    cv::GaussianBlur( dst, dst, {3, 3}, 0 );
    //    cv::threshold( dst, dst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );
    //    imageProcess( dst );
    //    cv::imshow( "dst", dst );
    //    cv::waitKey();
    //检测出最大的圆，暂时用霍夫圆检测方案
    vector< Vec3f > pcircles;//创建一个vector，存放圆的信息（[0] [1]圆心坐标，[2] 半径）
    //void HoughCircles(image， circles, method, dp, minDist, param1 = 100,param2 = 100, minRadius = 0, maxRadius = 0)
    //    const int min_radio = 70, max_radio = 80;
    // 好多参数是慢慢试出来的，感觉不靠谱e
    HoughCircles( dst, pcircles, HOUGH_GRADIENT_ALT, 1.0, 70, 250, 0.35, 70, 80 );//HOUGH_GRADIENT_ALT  HOUGH_GRADIENT
    //    RELEASE_ASSERT( pcircles.size() == 1 || pcircles.empty() ); //理论上，检测到圆的个数，应该只有0和1两种可能
    //没有检测到圆，直接返回NORMAL
    if( pcircles.empty() ) {
        //        printSentence( "We can't find a circle!!!" );
        return NORMAL;
    }
    Point  center;
    double radio = -1;
    for( const auto &vec: qAsConst( pcircles ) ) {
        if( radio < vec[ 2 ] ) {
            radio  = vec[ 2 ];
            center = Point( vec[ 0 ], vec[ 1 ] );
        }
    }
    Q_ASSERT( center.x > 0 && center.y > 0 );
    Q_ASSERT( radio > 0 );
    //将圆显示在BGR彩色图上，用以展示检测结果
    auto roi_bgr = bgr_image( rect );
    Q_ASSERT( !roi_bgr.empty() );
    Q_ASSERT( roi_bgr.channels() >= 3 );
    circle( roi_bgr, center, radio, Scalar( 0, 0, 255 ), 4, LINE_AA );//绘制圆(图片名，圆心位置，半径，颜色，边长)
    circle( roi_bgr, center, 2, Scalar( 0, 0, 255 ), 2, LINE_AA );    //绘制圆心
    //检测直线
    //    vector<Vec4f> lines;//存储直线的容器
    vector< Vec2f > lines;//存储直线的容器
    //用检测圆之前的图片来检测直线（因为两者预处理方案不同）
    dst = gray_image( rect ).clone();
    //把检测到的圆区域全部置白色，防止检测到圆内的黑色区域
    circle( dst, center, radio, Scalar( 255 ), -1 );//绘制圆(图片名，圆心位置，半径，颜色，边长)
    //    imageProcess( dst );
    //    cv::GaussianBlur( dst, dst, {3, 3}, 0 );
    //    cv::threshold( dst, dst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );
    //    cv::threshold( dst, dst, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU );
    cv::Canny( dst, dst, 100, 200 );
    //    cv::imshow( "dst", dst );  cv::waitKey();
    //    imshow( "dst", dst ); waitKey();
    cv::HoughLines( dst, lines, 2.0, CV_PI / 180.0 * 30, 50, 0, 0, 0, CV_PI / 180.0 * 120 );
    //    cv::HoughLinesP( dst, lines, 1.5, CV_PI / 180.0, 6, 0, 8 );
    //这里有疑惑，但暂时返回0，表示数据无效
    if( lines.empty() ) {
        //        printSentence( "We can't find a line!!!" );
//        return 0;
        return NORMAL;
    }
    //返回最大的 （圆心到直线距离）/圆半径 的比值
    double round_degree = 99999;
    for( const auto &elem: qAsConst( lines ) ) {
        //得到检测到直线上的两个点
        float     rho = elem[ 0 ], theta = elem[ 1 ];
        double    a = cos( theta ), b = sin( theta );
        double    x0 = a * rho, y0 = b * rho;
        cv::Point pt1 = cv::Point( cvRound( x0 + 1000 * ( -b ) ), cvRound( y0 + 1000 * ( a ) ) ),
                  pt2 = cv::Point( cvRound( x0 - 1000 * ( -b ) ), cvRound( y0 - 1000 * ( a ) ) );
        round_degree  = std::min( LineData( pt1, pt2 ).getDistance( center ) / radio, round_degree );
        line( roi_bgr, pt1, pt2, Scalar( 0, 255, 0 ), 3, LINE_AA );
    }
    std::string display_str = radio >= 70.0 && radio <= 80.0
                                  ? fmtStdStr( "%1", round_degree )
                                  : fmtStdStr( "%1 %2", round_degree, radio );
    OpenCVHelp::drawText( bgr_image, rect.tl() + Point( 0, -50 ), display_str, { 0, 255, 0 } );
    //    timer.printTime( TIMER_RECORDER_OUTPUT( "checkRound" ) );
    if( round_degree < 0.6 )
        return OVER_ONE_THIRD;
    else
        return NORMAL;
}

void FilmStripClassifier::imageProcess( cv::Mat &image, int threshold ) noexcept {
    cv::threshold( image, image, threshold, 255, cv::THRESH_BINARY );
    Q_ASSERT( !image.empty() );
}

const char *FilmStripClassifier::getEnumStr( const int iEnum ) noexcept {
    static constexpr const char *dst[] = { "NOT_PRESSED", "NORMAL", "OVER_ONE_THIRD", "NOT_PRESSED_WELL" };
    Q_ASSERT( iEnum >= 0 && iEnum < 4 );
    return dst[ iEnum ];
}
