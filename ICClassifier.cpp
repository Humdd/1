#include <execution>
#include <iostream>
#include "ICClassifier.h"
#include "ICGraphicsItem.h"
#include "ICAlgorithmParams.h"
#include "TimeRecorder.h"
#include "LineData.h"
#include "utils.h"
#include "ConsoleFileLogger.h"
#include <QMessageBox>

ICClassifier::ICClassifier() noexcept
    : image( params_ic.excute_gray_image )
    , image_BGR( params_ic.excute_bgr_image ) {
    Q_ASSERT( params_ic.isParamsValid() );
}

void ICClassifier::binaryThreshold( cv::Mat &gray_image, cv::Mat &rgb_image, const cv::Rect &rect, const int threshold ) noexcept {
    using namespace cv;
    Q_ASSERT( !gray_image.empty() );
    Q_ASSERT( !rgb_image.empty() );
    Mat roi_gray = gray_image( rect );
    Q_ASSERT( !roi_gray.empty() );
    imageProcess( roi_gray, threshold );
    auto roi_bgr = rgb_image( rect );
    cv::cvtColor( roi_gray, roi_bgr, COLOR_GRAY2RGB );
    Q_ASSERT( !roi_bgr.empty() );
    Q_ASSERT( roi_bgr.channels() >= 3 );
    //创建一个vector，存放圆的信息（[0] [1]圆心坐标，[2] 半径）
    std::vector< cv::Vec3f > pcircles;
    HoughCircles( gray_image, pcircles, cv::HOUGH_GRADIENT_ALT, 1.0, 70, 250, 0.35, 70, 80 );//HOUGH_GRADIENT_ALT  HOUGH_GRADIENT
    //    RELEASE_ASSERT( pcircles.size() == 1 || pcircles.empty() ); //理论上，检测到圆的个数，应该只有0和1两种可能
    //没有检测到圆,直接返回
    if( pcircles.empty() ) {
        printSentence( "We can't find a circle!!!" );
        return;
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
    circle( roi_bgr, center, radio, Scalar( 0, 0, 255 ), 4, LINE_AA );//绘制圆(图片名，圆心位置，半径，颜色，边长)
    circle( roi_bgr, center, 2, Scalar( 0, 0, 255 ), 2, LINE_AA );    //绘制圆心
}

int ICClassifier::execute1() noexcept {
    Q_ASSERT( params_ic.isParamsValid() );
    TimeRecorder timer;
    //    int          result = NORMAL;
    std::atomic_int result{ NORMAL };
    IGNORE_RESULT(
        std::find_if(
            std::execution::par_unseq, params_ic.rect_tuple_array.begin(), params_ic.rect_tuple_array.end(),
            [ this, &result ]( auto &&rect_tuple ) {
                const auto type = std::get< 3 >( rect_tuple );
                switch (type)
                {
                case ICGraphicsItem::RECT_CHIP:
                case ICGraphicsItem::RECT_LOGO:
                {
                    //                    Q_ASSERT( !std::get< 0 >( rect_var )->empty() );
                    std::atomic_int rect_result{checkRect(image, image_BGR, rect_tuple)};
                    if (result.load() == NORMAL && rect_result.load() != NORMAL)
                    {
                        result.store(rect_result);
                        return true;
                    }
                    else
                        return result.load() != NORMAL;
                }
                    break;
                case ICGraphicsItem::RECT_CIRCLE:
                {
                    return std::find_if(
                               std::execution::par_unseq,
                               params_ic.rect_circle_array.begin(),
                               params_ic.rect_circle_array.end(),
                               [this, &result](auto &&circle)
                               {
                                   std::atomic_int circle_result{ checkRound( *circle, image, image_BGR ) };
                                   if( result.load() == NORMAL && circle_result.load() != NORMAL ) {
                                       result.store( circle_result );
                                       return true;
                                   }
                                   else
                                       return result.load() != NORMAL; }) != params_ic.rect_circle_array.end();
                }
                    break;
                default:
                    exit(3);
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
    params_ic.algorithmFinished();
    return result.load();
}

cv::Rect getRectWithPadding(const cv::Rect &base, const cv::Rect &rect, int pad)
{
    int x = rect.x - pad < base.x ? base.x : rect.x - pad; //新坐标
    int y = rect.y - pad < base.y ? base.y : rect.y - pad;
    int width = x + rect.width + 2 * pad > base.x + base.width ? base.x + base.width - x : rect.width + 2 * pad;
    int height = y + rect.height + 2 * pad > base.y + base.height ? base.y + base.height - y : rect.height + 2 * pad;
    Q_ASSERT(width > 0 && height > 0);
    return cv::Rect(x, y, width, height);
}

cv::Rect getRelativeRect(const cv::Rect &base, const cv::Rect &rect)
{
    //求rect在base内部的位置
    int x = rect.x - base.x;
    int y = rect.y - base.y;
    int width = rect.width;
    int height = rect.height;
    int edge = width > height ? width : height;
    return cv::Rect(x, y, edge, edge);
}

cv::Rect getRotateRect(const cv::Rect &base, const cv::Rect &rect)
{
    //求rect围绕base中点旋转后的位置，请确保rect在base内部
    cv::Rect rect_relative = getRelativeRect(base, rect);

    int x_middle = base.width / 2;
    int y_middle = base.height / 2;
    int x_new = 2 * x_middle - rect_relative.x - rect_relative.width; //相对于base的新位置
    int y_new = 2 * y_middle - rect_relative.y - rect_relative.height;
    return cv::Rect(x_new + base.x, y_new + base.y, rect_relative.width, rect_relative.height); //返回相对于原图的位置
}

//1 检测朝向
//2 如果朝向正常
//2.1 检测logo
//2.2 检测文字

int ICClassifier::execute() noexcept
{
    Q_ASSERT(params_ic.isParamsValid());
    NoteTimingStart;
//    TimeRecorder timer;
    std::atomic_int result{NORMAL};

    //1 检测朝向
    //检测圆的数量 rect_chip、rect_circle、rect_circle_rotate
//    int cnt_BGA_circle = 30;
//    int minDist = 60;
//    int cannyThresh = 120;
//    int precision = 17;
//    int minRadius_bga = 12; //BGA 14
//    int maxRadius_bga = 17; //BGA 17
//    int minRadius = 26; //circle 26
//    int maxRadius = 31; //circle 31
//    int pad_circle = 60;
    const auto &[ cnt_BGA_circle, minDist, cannyThresh, precision, minRadius_bga, maxRadius_bga, minRadius, maxRadius, pad_circle ] = params_ic.circle_params;

    //检测bga
    auto rect_chip_ptr = std::get<0>(params_ic.rect_tuple_array[0]);
    if (!rect_chip_ptr)
    {
        QMessageBox::critical(nullptr, u8"错误", u8"圆点矩形异常，请重新标注");
        params_ic.algorithmFinished();
        return -1;
    }
    cv::Rect rect_chip = *rect_chip_ptr;
    int cnt_chip = detectCircle(rect_chip, image, image_BGR, minDist, cannyThresh, precision, minRadius_bga, maxRadius_bga, FLIPPED);

    //检测正常朝向
    auto rect_circle_ptr = std::get<0>(params_ic.rect_tuple_array[2]);
    if (!rect_circle_ptr)
    {
        QMessageBox::critical(nullptr, u8"错误", u8"圆点矩形异常，请重新标注");
        params_ic.algorithmFinished();
        return -1;
    }
    cv::Rect rect_circle = *rect_circle_ptr;
    cv::Rect rect_circle_pad = getRectWithPadding(rect_chip, rect_circle, pad_circle);
    int cnt_circle = detectCircle(rect_circle_pad, image, image_BGR, minDist, cannyThresh, precision, minRadius, maxRadius, NORMAL);

    //检测旋转朝向
    cv::Rect rect_circle_rotate = getRotateRect(rect_chip, rect_circle);
    cv::Rect rect_circle_rotate_pad = getRectWithPadding(rect_chip, rect_circle_rotate, pad_circle);
    int cnt_circle_rotate = detectCircle(rect_circle_rotate_pad, image, image_BGR, minDist, cannyThresh, precision, minRadius, maxRadius, ROTATED);

    //根据霍夫检测圆的数量判断芯片朝向
    if (cnt_chip > cnt_BGA_circle)
    {
        std::cout << "[Flipped] " << "cnt_chip: " << cnt_chip << std::endl;
        result.store(FLIPPED);
    }
    else if (cnt_circle > 0)
    {
        std::cout << "[normal] " << "cnt_circle: " << cnt_circle << std::endl;
        result.store(NORMAL);
    }
    else if (cnt_circle_rotate > 0)
    {
        std::cout << "[rotate] " << "cnt_circle_rotate: " << cnt_circle_rotate << std::endl;
        result.store(ROTATED);
    }
    else
    {
        std::cout << "[Empty] " << "cnt_chip_img: " << cnt_chip << std::endl;
        result.store(EMPTY);
    }

    //2 朝向结果正常才检测logo
    if (result.load() == ICClassifier::NORMAL)
    {
        //获取logo
        const auto &[rect_logo_ptr, binary_threshold, wnd_size, type] = params_ic.rect_tuple_array[1];
        const auto &[ diffRatio, wndsz, pad_logo ] = params_ic.logo_params;
        if (!rect_logo_ptr)
        {
            QMessageBox::critical(nullptr, u8"错误", u8"logo矩形异常，请重新标注");
            params_ic.algorithmFinished();
            return -1;
        }
        cv::Rect rect_logo = *rect_logo_ptr;
        cv::Rect rect_logo_pad = cv::Rect(rect_logo.x - pad_logo, rect_logo.y - pad_logo, rect_logo.width + 2 * pad_logo, rect_logo.height + 2 * pad_logo);
        cv::Mat mat_logo = image(rect_logo_pad);
        //获取模板，并判断是否存在
        QString saveDir("./images/templ/");
        cv::Mat templ = cv::imread(QString("%1%2").arg(saveDir, "templ.bmp").toStdString(), cv::IMREAD_GRAYSCALE);
        cv::Mat templ_bin = cv::imread(QString("%1%2").arg(saveDir, "templ_bin.bmp").toStdString(), cv::IMREAD_GRAYSCALE);
        cv::Mat templ_bin_mask = cv::imread(QString("%1%2").arg(saveDir, "templ_bin_mask.bmp").toStdString(), cv::IMREAD_GRAYSCALE);
        if (templ.empty() || templ_bin.empty() || templ_bin_mask.empty())
        {
            QMessageBox::critical(nullptr, u8"错误", QString(u8"%1 模板图像不存在，请重新标注！").arg(saveDir));
            return -1;
        }
        //开始检测
        Utils utils;
        cv::Rect rect_matchResult = utils.getMatchResultROI(mat_logo, templ);
        cv::Mat matchResult = mat_logo(rect_matchResult);
        bool isAbnormal = utils.inspectLogo(matchResult, templ, templ_bin, templ_bin_mask, binary_threshold, wndsz);
        if (isAbnormal)
        {
            result.store(LOGO_ABNORMAL);
        }
    }

    //3 在BGR图上，显示朝向检测结果
    cv::Mat roi_bgr = image_BGR(rect_chip);
    Q_ASSERT(!roi_bgr.empty());
    Q_ASSERT(roi_bgr.channels() >= 3);
    cv::Scalar color;
    switch(result.load())
    {
    case ICClassifier::EMPTY:
        color = cv::Scalar(0, 0, 255); //红
        break;
    case ICClassifier::NORMAL:
        color = cv::Scalar(0, 255, 0); //绿
        break;
    case ICClassifier::ROTATED:
        color = cv::Scalar(255, 0, 0); //蓝
        break;
    case ICClassifier::FLIPPED:
        color = cv::Scalar(0, 255, 255); //黄
        break;
    case ICClassifier::LOGO_ABNORMAL:
        color = cv::Scalar(255, 255, 0); //青
        break;
    default:
        color = cv::Scalar(255, 255, 255);
        break;
    }
    cv::rectangle(image_BGR, rect_chip, color, 14, cv::LINE_AA); //绘制朝向检测结果


//    cv::imshow("rect_circle_pad", image(rect_circle_pad));
//    cv::imshow("rect_circle_rotate_pad", image(rect_circle_rotate_pad));
//    cv::waitKey(0);
    params_ic.algorithmFinished();
//    timer.printTime(TIMER_RECORDER_OUTPUT("excute"));
//    Q_LOG4CPLUS_INFO(TIMER_RECORDER_OUTPUT("excute"))
    return result.load();
}

//#include <QThread>
void ICClassifier::train() noexcept {
    //更新图像
    params_ic.resetParams();
//    TIMER_RECORD_FUNC_TIME;
//    std::for_each(params_ic.rect_tuple_array.begin(), params_ic.rect_tuple_array.end(), [](){});
    std::for_each(
        std::execution::seq, params_ic.rect_tuple_array.begin(), params_ic.rect_tuple_array.end(),
        []( auto &&rect_tuple ) {
            //            printVar( QThread::currentThreadId() );
            auto &[ rect_ptr, binary_threshold, mean_height, type ] = rect_tuple;
            if (params_ic.rect_binary_threshold_ptr && *params_ic.rect_binary_threshold_ptr != binary_threshold)
                return;
            switch( type ) {
                case ICGraphicsItem::RECT_CHIP:
                    break;
                case ICGraphicsItem::RECT_LOGO: {
                    if( !rect_ptr ) return;
                    const auto white_point_count = ICClassifier::getMeanHeight( params_ic.excute_gray_image, params_ic.excute_bgr_image, rect_tuple ).first;
//                    mean_height = rect_ptr->height - white_point_count / double( rect_ptr->width );
//                    mean_height = 6; //暂时指定为判断窗口的大小，命名后续再更改为 wnd_size
                } break;
                case ICGraphicsItem::RECT_CIRCLE:
//                    std::for_each(
//                        std::execution::par_unseq, params_ic.rect_circle_array.begin(), params_ic.rect_circle_array.end(),
//                        []( auto &&circle_rect_ptr ) {
//                            Q_ASSERT( circle_rect_ptr );
//                            const auto &rect = *circle_rect_ptr;
//                            Q_ASSERT( !rect.empty() );
//                            ICClassifier::checkRound( rect, params_ic.excute_gray_image, params_ic.excute_bgr_image );
//                        } );
                    break;
            }
        } );
}

std::pair< int, cv::Mat > ICClassifier::getMeanHeight(
    cv::Mat &gray_image, cv::Mat &bgr_image,
    const ICAlgorithmParams::ParamsTupleType &rect_tuple ) noexcept
{
    //    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( gray_image.channels() == 1 );
    Q_ASSERT( bgr_image.channels() == 3 );
    Q_ASSERT( params_ic.status == ICAlgorithmParams::SET_EXECUTE_IMG );
    const auto &[ rect_ptr, threshold, _, type ] = rect_tuple;
    if (!rect_ptr)
    {
        QMessageBox::critical(nullptr, u8"错误", u8"rect_ptr为空");
    }
    const auto &rect                             = *rect_ptr;
    printVar(rect); //rect也正常
//    cv::imshow("gray_image", gray_image);
//    cv::imshow("bgr_image", bgr_image);
//    cv::waitKey(0);
    cv::Mat     roi                              = gray_image( rect ).clone(); //bug，排除rect_ptr是空指针
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

int ICClassifier::checkRect( cv::Mat &gray_image, cv::Mat &bgr_image, const ICAlgorithmParams::ParamsTupleType &rect_tuple ) noexcept {
    Q_ASSERT( gray_image.channels() == 1 );
    Q_ASSERT( bgr_image.channels() == 3 );
    Q_ASSERT( !bgr_image.empty() );
    using namespace std;
    using namespace cv;
    TimeRecorder timer;
    int          result                                 = NORMAL;
    const auto &[ rect_ptr, threshold, mean_height, _ ] = rect_tuple;
    const auto &rect                                    = *rect_ptr;
    const int   max_threshold                           = rect.area() * params_ic.max_black_point_ratio;
    std::string display_str;
    Scalar      scalar;
    const auto &[ roi_white_count, roi ] = getMeanHeight( gray_image, bgr_image, rect_tuple );
    const auto current_mean_height       = rect.height - roi_white_count / double( rect.width );
    if( roi_white_count >= 0.95 * rect.area() ) {
        //        printVar( roi_white_count );
        //        printVar( rect.area() );
        //        printVar( roi_white_count / ( rect.area() * 1.0 ) );
        //        printSeparate;
        result      = FLIPPED;
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
        const int invalid_col_threshold   = params_ic.invalid_col_ratio * rect.width;
        int       invalid_point_threshold = params_ic.invalid_point_ratio * mean_height;
        for( int col = 0; col < roi.cols; ++col ) {
            //获取某一列
            auto &&col_mat = roi.colRange( col, col + 1 );
            Q_ASSERT( !col_mat.empty() );
            //统计该列黑点数量
            int black_point_count = roi.rows - cv::countNonZero( col_mat );
            //        printVar( black_point_count );
            if( black_point_count < invalid_point_threshold &&
                ++invalid_col_count >= invalid_col_threshold ) {
                result = ROTATED;
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

int ICClassifier::checkRound( const cv::Rect &rect, cv::Mat &gray_image, cv::Mat &bgr_image ) noexcept {
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
        return 0;
    }
    //返回最大的 （圆心到直线距离）/圆半径 的比值
    double round_degree = 99999;
    for( const auto &elem: qAsConst( lines ) ) {
        //得到检测到直线上的两个点
        float  rho = elem[ 0 ], theta = elem[ 1 ];
        double a = cos( theta ), b = sin( theta );
        double x0 = a * rho, y0 = b * rho;
        Point  pt1   = Point( cvRound( x0 + 1000 * ( -b ) ), cvRound( y0 + 1000 * ( a ) ) ),
              pt2    = Point( cvRound( x0 - 1000 * ( -b ) ), cvRound( y0 - 1000 * ( a ) ) );
        round_degree = std::min( LineData( pt1, pt2 ).getDistance( center ) / radio, round_degree );
        line( roi_bgr, pt1, pt2, Scalar( 0, 255, 0 ), 3, LINE_AA );
    }
    std::string display_str = radio >= 70.0 && radio <= 80.0
                                  ? fmtStdStr( "%1", round_degree )
                                  : fmtStdStr( "%1 %2", round_degree, radio );
    OpenCVHelp::drawText( bgr_image, rect.tl() + Point( 0, -50 ), display_str, { 0, 255, 0 } );
    //    timer.printTime( TIMER_RECORDER_OUTPUT( "checkRound" ) );
    if( round_degree < 0.6 )
        return EMPTY;
    else
        return NORMAL;
}

int ICClassifier::detectCircle(const cv::Rect &rect, cv::Mat &image_gray, cv::Mat &image_bgr, int minDist, int cannyThresh, int precision, int minRadius, int maxRadius, ChipStatus chipStatus) noexcept
{
    using namespace std;
    using namespace cv;
    Q_ASSERT(!image_gray.empty());
    Q_ASSERT(!image_bgr.empty());
    Q_ASSERT(image_gray.channels() == 1);
    Q_ASSERT(image_bgr.channels() == 3);
    TimeRecorder timer;
    Mat dst = image_gray(rect);
    Q_ASSERT(!dst.empty());
//    cv::GaussianBlur( dst, dst, {3, 3}, 0 );
//    cv::threshold( dst, dst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );
//    imageProcess( dst );
//    cv::imshow( "dst", dst );
//    cv::waitKey();
    //检测出最大的圆，用霍夫圆检测方案
    vector<Vec3f> circles; //创建一个vector，存放圆的信息（[0] [1]圆心坐标，[2] 半径）
//    cv::HoughCircles(dst, circles, cv::HOUGH_GRADIENT, 1, 62, 200, 16, 25, 31);
    cv::HoughCircles(dst, circles, cv::HOUGH_GRADIENT, 1, minDist, cannyThresh, precision, minRadius, maxRadius);
    //没有检测到圆，直接返回检测结果
    if (circles.empty())
    {
        qDebug() << "No circle detected";
#ifdef _DEBUG
        if (chipStatus == NORMAL)
        {
            cv::imwrite("D:/bin/roi_NORMAL.bmp", dst);

        }
#endif
        return EMPTY;
    }
    //将圆显示在BGR彩色图上，用以展示检测结果
    Mat roi_bgr = image_bgr(rect);
    Q_ASSERT(!roi_bgr.empty());
    Q_ASSERT(roi_bgr.channels() >= 3);
    cv::Scalar color;
    switch(chipStatus)
    {
    case ICClassifier::EMPTY:
        color = cv::Scalar(0, 0, 255); //红
        break;
    case ICClassifier::NORMAL:
        color = cv::Scalar(0, 255, 0); //绿
        break;
    case ICClassifier::ROTATED:
        color = cv::Scalar(255, 0, 0); //蓝
        break;
    case ICClassifier::FLIPPED:
        color = cv::Scalar(0, 255, 255); //黄
        break;
    case ICClassifier::LOGO_ABNORMAL:
        color = cv::Scalar(255, 255, 0); //青
        break;
    default:
        color = cv::Scalar(255, 255, 255);
        break;
    }

    for (const auto &vec : qAsConst(circles))
    {
        auto radius = vec[2];
        Point center = Point(vec[0], vec[1]);
        circle(roi_bgr, center, radius, color, 4, LINE_AA); //绘制圆(图像，圆心位置，半径，颜色，边长)
#ifdef _DEBUG
        if (chipStatus == NORMAL)
        {
            qDebug() << "radius:" << radius;
        }
#endif
    }

#ifdef _DEBUG
    qDebug() << "total circles: " << circles.size();
    if (chipStatus == NORMAL)
    {
        cv::imshow("Hough Circle", roi_bgr);
        cv::imwrite("D:/bin/roi_bgr.bmp", roi_bgr);
        cv::waitKey(0);
    }
#endif

    return circles.size();
}

//not used
int ICClassifier::detectCircle(const cv::Mat &src, int minDist, int cannyThresh, int precision, int minRadius, int maxRadius) noexcept
{
    // ###################  Hough Circle  ###########################
//    cv::resize(src, src, cv::Size(0, 0), 0.5, 0.5);
    cv::Mat src_show;
    cv::cvtColor(src, src_show, cv::COLOR_GRAY2BGR);

    auto time_start = std::chrono::system_clock::now();
    std::vector<cv::Vec3f> circles;
    //image, circles, method, dp, minDist(圆心间最小距离), param1(Canny阈值), param2(越大对圆的要求越高), minRadius, maxRadius
//    cv::HoughCircles(src, circles, cv::HOUGH_GRADIENT, 1, 16, 200, 16, 4, 8); // 3  diameter [14,16]
    cv::HoughCircles(src, circles, cv::HOUGH_GRADIENT, 1, minDist, cannyThresh, precision, minRadius, maxRadius);

    for (int i = 0; i < circles.size(); ++i) {
        cv::Vec3f c = circles.at(i);
        cv::Point center(c[0], c[1]);
        float radius = c[2];
#ifdef _DEBUG
        std::cout << radius << std::endl;
#endif
        cv::circle(src_show, center, radius, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }
#ifdef _DEBUG
    std::cout << "total circles: " << circles.size() << std::endl;
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start);
    std::cout << "[Elapsed time]: " << duration.count() << " ms" << std::endl;
    cv::destroyAllWindows();
    cv::imshow("Hough Circle", src_show);
    cv::waitKey(0);
#endif
    // ###################  Hough Circle  ###########################
    return circles.size();
}

void ICClassifier::imageProcess( cv::Mat &image, int threshold ) noexcept {
    cv::threshold( image, image, threshold, 255, cv::THRESH_BINARY );
    Q_ASSERT( !image.empty() );
}

const char *ICClassifier::getEnumStr( const int iEnum ) noexcept {
    static constexpr const char *dst[] = { "NORMAL", "OVER_ONE_THIRD", "NOT_PRESSED_WELL", "NOT_PRESSED" };
    Q_ASSERT( iEnum >= 0 && iEnum < 4 );
    return dst[ iEnum ];
}
