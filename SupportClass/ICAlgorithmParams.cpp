#include "ICAlgorithmParams.h"
#include "ICGraphicsScene.h"
#include "ICAlgorithmPropertBrowserWindow.h"
#include "ICAlgorithmPropertBrowser.h"
#include "TimeRecorder.h"
#include <QMessageBox>

//IC相机参数类
ICAlgorithmParams::ICAlgorithmParams() noexcept
    :
    logo_params{0.97, 10, 60},
    circle_params{30, 60, 120, 13, 12, 17, 26, 30, 60},
    rect_tuple_array{
        ParamsTupleType{ nullptr, 70, 40, 1 },
        ParamsTupleType{ nullptr, 120, 8, 2 },
        ParamsTupleType{ nullptr, 26, 30, 3 },
    }
{}

//
QString ICAlgorithmParams::paramRectStatus() noexcept {
    constexpr static std::array< const char *, 5 > tmp = {
        u8"上矩形框||", u8"下矩形框||", u8"小圆圈框||",
        u8"上矩形框黑柱平均宽度||", u8"下矩形框黑柱平均宽度||"
    };
    const auto &[ rect_chip_ptr, rect_chip_binary_threshold, rect_chip_mean_height, rect_chip_type ] = rect_tuple_array.at( 0 );
    const auto &[ rect_logo_ptr, rect_logo_binary_threshold, rect_logo_mean_height, rect_logo_type ] = rect_tuple_array.at( 1 );
    const std::array< bool, 5 > dst_help                                                             = {
        !rect_chip_ptr,
        !rect_logo_ptr,
        rect_circle_array.isEmpty(),
        !rect_chip_mean_height,
        !rect_logo_mean_height,
    };
    QString dst;
    dst.reserve( 7 * 3 + 13 * 2 );
    for( int index = 0; index < dst_help.size(); ++index ) {
        if( dst_help.at( index ) ) {
            dst.append( tmp.at( index ) );
        }
    }
    return dst;
}

//更新图片
void ICAlgorithmParams::resetOriginImage( cv::Mat origin_gray_image, cv::Mat origin_bgr_image ) noexcept {
    Q_ASSERT( !origin_gray_image.empty() );
    Q_ASSERT( !origin_bgr_image.empty() );
    ICAlgorithmParams::origin_gray_image = origin_gray_image;
    ICAlgorithmParams::origin_bgr_image  = origin_bgr_image;
    status                               = SET_ORIGIN_IMG;
    //    resetParams();
}


bool ICAlgorithmParams::isStarted() noexcept {
    return status != EMPTY;
}

void ICAlgorithmParams::resetParams() noexcept {
    Q_ASSERT( status != EMPTY );
    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( !origin_gray_image.empty() && !origin_bgr_image.empty() );
    excute_gray_image = origin_gray_image;
    excute_bgr_image  = origin_bgr_image.clone();
    Q_ASSERT( !excute_gray_image.empty() && !excute_bgr_image.empty() );
    status = SET_EXECUTE_IMG;
}

bool ICAlgorithmParams::isParamsValid() noexcept {
    const auto &[ rect_chip_ptr, rect_chip_binary_threshold, rect_chip_mean_height, _1 ] = rect_tuple_array.at( 0 );
    const auto &[ rect_logo_ptr, rect_logo_binary_threshold, rect_logo_mean_height, _2 ] = rect_tuple_array.at( 1 );
    Q_UNUSED( rect_chip_binary_threshold )
    Q_UNUSED( rect_logo_binary_threshold )
    return rect_chip_ptr && rect_logo_ptr &&
           rect_chip_mean_height > 0 && rect_logo_mean_height > 0 &&
           status == SET_EXECUTE_IMG;
}

void ICAlgorithmParams::algorithmFinished() noexcept {
    Q_ASSERT( status == SET_EXECUTE_IMG );
    status = ALGORITHM_FINISHED;
}

//读取json文件
void ICAlgorithmParams::getParamsFromJson() noexcept {
    //遍历属性列表
    //函数getSubClassBrowser遍历AbstractPropertyBrowserWindow的子类列表，并判断每个子类是否是类型为ICAlgorithmPropertBrowser的属性浏览器对象，如果是，则返回该对象的指针；否则，返回 nullptr。
    static auto *algorithm_browser = ICAlgorithmPropertBrowserWindow::getInstance().getSubClassBrowser< ICAlgorithmPropertBrowser >();
    //检查特定的条件是否成立。如果断言失败，会弹出一个错误对话框，提示开发者程序出现了问题。
    Q_ASSERT( algorithm_browser );
    //将json对象中的某个属性值转换为对应的C++数据类型
    algorithm_browser->setTableFromJson();
    static auto &scene = ICGraphicsScene::getInstance();
    //rect_tuple_array存储对应参数值
    auto &[ _1, rect_chip_binary_threshold, rect_chip_mean_height, _2 ] = rect_tuple_array.at( 0 );
    auto &[ _3, rect_logo_binary_threshold, rect_logo_wndsz, _4 ]       = rect_tuple_array.at( 1 );
    auto &[ _5, rect_circle_minRadius, rect_circle_maxRadius, _6 ]      = rect_tuple_array.at( 2 );
    auto &[ diffRatio, wndsz, pad_logo ] = logo_params;
    auto &[ cnt_BGA_circle, minDist, cannyThresh, precision, minRadius_bga, maxRadius_bga, minRadius, maxRadius, pad_circle ] = circle_params;
    //从algorithm_browser获取参数
    auto rect1_str             = algorithm_browser->getJsonValue<QString>(0);
    rect_chip_binary_threshold = algorithm_browser->getJsonValue<int>(1);
    rect_chip_mean_height      = algorithm_browser->getJsonValue<int>(2);
    auto rect2_str             = algorithm_browser->getJsonValue<QString>(3);
    rect_logo_binary_threshold = algorithm_browser->getJsonValue<int>(4);
    diffRatio                  = algorithm_browser->getJsonValue<double>(5);
    wndsz                      = algorithm_browser->getJsonValue<int>(6);
    pad_logo                   = algorithm_browser->getJsonValue<int>(7);
    auto rect3_str             = algorithm_browser->getJsonValue<QString >(8);
    cnt_BGA_circle             = algorithm_browser->getJsonValue<int>(9);
    minDist                    = algorithm_browser->getJsonValue<int>(10);
    cannyThresh                = algorithm_browser->getJsonValue<int>(11);
    precision                  = algorithm_browser->getJsonValue<int>(12);
    minRadius_bga              = algorithm_browser->getJsonValue<int>(13);
    maxRadius_bga              = algorithm_browser->getJsonValue<int>(14);
    minRadius                  = algorithm_browser->getJsonValue<int>(15);
    maxRadius                  = algorithm_browser->getJsonValue<int>(16);
    pad_circle                 = algorithm_browser->getJsonValue<int>(17);

    //判空
    if( !rect1_str.isEmpty() || !rect2_str.isEmpty() || !rect3_str.isEmpty() )
        scene.clear();
    else
        return;
    if( !rect1_str.isEmpty() ) {
        const auto &point_array = StringOperation::str2NumArray< int >( rect1_str );
        Q_ASSERT( point_array.size() == 4 );

        scene.addAbstractItem(
            new ICGraphicsItem(
                QRectF( point_array.at( 0 ), point_array.at( 1 ), point_array.at( 2 ), point_array.at( 3 ) ),
                ICGraphicsItem::RECT_CHIP ),
            ICGraphicsItem::RECT_CHIP );
    }
    if( !rect2_str.isEmpty() ) {
        const auto &point_array = StringOperation::str2NumArray< int >( rect2_str );
        Q_ASSERT( point_array.size() == 4 );
        scene.addAbstractItem(
            new ICGraphicsItem(
                QRectF( point_array.at( 0 ), point_array.at( 1 ), point_array.at( 2 ), point_array.at( 3 ) ),
                ICGraphicsItem::RECT_LOGO ),
            ICGraphicsItem::RECT_LOGO );
    }
    if( !rect3_str.isEmpty() ) {
        const auto &rect3_str_list = rect3_str.replace( "[", "" ).replace( "]", "" ).split( "||" );
        for( const auto &fact_rect3_str: qAsConst( rect3_str_list ) ) {
            const auto &point_array = StringOperation::str2NumArray< int >( fact_rect3_str );
            Q_ASSERT( point_array.size() == 4 );
            scene.addAbstractItem(
                new ICGraphicsItem(
                    QRectF( point_array.at( 0 ), point_array.at( 1 ), point_array.at( 2 ), point_array.at( 3 ) ),
                    ICGraphicsItem::RECT_CIRCLE ),
                ICGraphicsItem::RECT_CIRCLE );
        }
    }
}

//保存json文件
void ICAlgorithmParams::storeParamsIntoJson() noexcept {
    printFuncInfo;
    if( status == EMPTY )
        return;
    QString rect1_str, rect2_str, rect3_str;//分别是chip、logo、circle
    const auto &[ rect_chip_ptr, rect_chip_binary_threshold, rect_chip_mean_height, _1 ]       = rect_tuple_array.at( 0 );
    const auto &[ rect_logo_ptr, rect_logo_binary_threshold, rect_logo_mean_height, _2 ]       = rect_tuple_array.at( 1 );
    const auto &[ rect_circle_ptr, rect_circle_binary_threshold, rect_circle_mean_height, _3 ] = rect_tuple_array.at( 2 );
    const auto &[ diffRatio, wndsz, pad_logo ] = logo_params;
    auto &[ cnt_BGA_circle, minDist, cannyThresh, precision, minRadius_bga, maxRadius_bga, minRadius, maxRadius, pad_circle ] = circle_params;

    if( rect_chip_ptr ) {
        const auto &rect = *rect_chip_ptr;
        Q_ASSERT( !rect.empty() );
        const QVector< int > &tmp = { rect.x, rect.y, rect.width, rect.height };
        //数字数组转换为字符串的函数
        rect1_str                 = StringOperation::numArray2Str( tmp );
    }

    if( rect_logo_ptr ) {
        const auto &rect = *rect_logo_ptr;
        //        printVar( rect_logo_ptr );
        //        printVar( rect );
        Q_ASSERT( !rect.empty() );
        const QVector< int > &tmp = { rect.x, rect.y, rect.width, rect.height };
        rect2_str                 = StringOperation::numArray2Str( tmp );

        QString saveDir( "./images/templ/" );
        //创建保存图像的文件夹
        if( !QDir().exists( saveDir ) ) {
            QDir().mkpath( saveDir );
        }
        //保存templ
        cv::Rect rect_logo = *rect_logo_ptr;
        cv::Mat  templ     = origin_gray_image( rect_logo );
        cv::imwrite( QString( "%1%2" ).arg( saveDir, "templ.bmp" ).toStdString(), templ );

        //保存templ_bin
        cv::Mat templ_bin;
        cv::threshold( templ, templ_bin, rect_logo_binary_threshold, 255, cv::THRESH_OTSU );
        cv::imwrite( QString( "%1%2" ).arg( saveDir, "templ_bin.bmp" ).toStdString(), templ_bin );

        //保存templ_bin_mask
        cv::Mat                                 templ_bin_mask;
        std::vector< std::vector< cv::Point > > contours;
        std::vector< cv::Vec4i >                hierarchy;
        cv::findContours( templ_bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );
        cv::Mat templ_bin_contours = cv::Mat::zeros( templ_bin.size(), CV_8UC3 );// 彩色轮廓使用3通道 CV_8UC3
        for( size_t i = 0; i < contours.size(); i++ ) {
            cv::drawContours( templ_bin_contours, contours, ( int )i, cv::Scalar( 255, 255, 255 ), -1, 8, hierarchy );
        }
        cv::cvtColor( templ_bin_contours, templ_bin_mask, cv::COLOR_BGR2GRAY );
        cv::imwrite( QString( "%1%2" ).arg( saveDir, "templ_bin_mask.bmp" ).toStdString(), templ_bin_mask );
    }

    if( rect_circle_ptr ) {
        const auto &rect = *rect_circle_ptr;
        //        printVar( rect_circle_ptr );
        //        printVar( rect );
        Q_ASSERT( !rect.empty() );
        const QVector< int > &tmp = { rect.x, rect.y, rect.width, rect.height };
        rect3_str                 = StringOperation::numArray2Str( tmp );
        minRadius = (rect.width + rect.height) / 2 / 2 - 4; //
        maxRadius = (rect.width + rect.height) / 2 / 2 + 2; // +2是扩大一点点
    }

    static auto *algorithm_browser = ICAlgorithmPropertBrowserWindow::getInstance().getSubClassBrowser< ICAlgorithmPropertBrowser >();
    Q_ASSERT( algorithm_browser );

    algorithm_browser->flushTableValue( 0, rect1_str );//chip
    algorithm_browser->flushTableValue( 1, rect_chip_binary_threshold );
    algorithm_browser->flushTableValue( 2, rect_chip_mean_height );
    algorithm_browser->flushTableValue( 3, rect2_str );//logo
    algorithm_browser->flushTableValue( 4, rect_logo_binary_threshold );
    algorithm_browser->flushTableValue( 5, diffRatio );
    algorithm_browser->flushTableValue( 6, wndsz );
    algorithm_browser->flushTableValue( 7, pad_logo );
    algorithm_browser->flushTableValue( 8, rect3_str );//circle
    algorithm_browser->flushTableValue( 9, cnt_BGA_circle );
    algorithm_browser->flushTableValue( 10, minDist );
    algorithm_browser->flushTableValue( 11, cannyThresh );
    algorithm_browser->flushTableValue( 12, precision );
    algorithm_browser->flushTableValue( 13, minRadius_bga );
    algorithm_browser->flushTableValue( 14, maxRadius_bga );
    algorithm_browser->flushTableValue( 15, minRadius );
    algorithm_browser->flushTableValue( 16, maxRadius );
    algorithm_browser->flushTableValue( 17, pad_circle );
    algorithm_browser->setJsonFromTable();

    //提示保存成功
    QMessageBox::information( nullptr, u8"提示", u8"保存成功" );
}

void ICAlgorithmParams::resetOriginGrayImage( QImage img ) noexcept {
    TIMER_RECORD_FUNC_TIME;
    origin_gray_image = excute_gray_image = Cv2Qt::QImage2Mat( img ).clone(); //需要clone
    Q_ASSERT( !origin_gray_image.empty() );
    cv::cvtColor( origin_gray_image, origin_bgr_image, cv::COLOR_GRAY2RGB );
    excute_bgr_image = origin_bgr_image;
    status           = SET_EXECUTE_IMG;
}

void ICAlgorithmParams::resetOriginGrayImage( cv::Mat img ) noexcept {
    //    TIMER_RECORD_FUNC_TIME;
    origin_gray_image = excute_gray_image = img.clone();
    Q_ASSERT( !origin_gray_image.empty() );
    cv::cvtColor( origin_gray_image, origin_bgr_image, cv::COLOR_GRAY2RGB );
    excute_bgr_image = origin_bgr_image;
    status           = SET_EXECUTE_IMG;
}
