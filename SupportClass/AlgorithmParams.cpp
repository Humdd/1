#include "AlgorithmParams.h"
#include "AbstractGraphicsScene.h"
#include "AlgorithmPropertBrowserWindow.h"
#include "AlgorithmPropertBrowser.h"
#include <QMessageBox>
#include "TimeRecorder.h"

//热合相机参数类
AlgorithmParams::AlgorithmParams() noexcept
    : rect_tuple_array{
        ParamsTupleType{ nullptr, 70, 40, 1 },
        ParamsTupleType{ nullptr, 70, 40, 2 },
        ParamsTupleType{ nullptr, 70, 40, 3 },
    } {}

//上下矩形的区域
QString AlgorithmParams::paramRectStatus() noexcept {
    constexpr static std::array< const char *, 5 > tmp = {
        u8"上矩形框||", u8"下矩形框||", u8"小圆圈框||",
        u8"上矩形框黑柱平均宽度||", u8"下矩形框黑柱平均宽度||"
    };
    const auto &[ rect_up_ptr, rect_up_binary_threshold, rect_up_mean_height, rect_up_type ]         = rect_tuple_array.at( 0 );
    const auto &[ rect_down_ptr, rect_down_binary_threshold, rect_down_mean_height, rect_down_type ] = rect_tuple_array.at( 1 );
    const std::array< bool, 5 > dst_help                                                             = {
        !rect_up_ptr,
        !rect_down_ptr,
        rect_circle_array.isEmpty(),
        !rect_up_mean_height,
        !rect_down_mean_height,
    };
    QString dst;
    //分配额外的内存
    dst.reserve( 7 * 3 + 13 * 2 );
    for( int index = 0; index < dst_help.size(); ++index ) {
        if( dst_help.at( index ) ) {
            dst.append( tmp.at( index ) );
        }
    }
    return dst;
}

//更新图片
void AlgorithmParams::resetOriginImage( cv::Mat origin_gray_image, cv::Mat origin_bgr_image ) noexcept {
    Q_ASSERT( !origin_gray_image.empty() );
    Q_ASSERT( !origin_bgr_image.empty() );
    AlgorithmParams::origin_gray_image = origin_gray_image;
    AlgorithmParams::origin_bgr_image  = origin_bgr_image;
    status                             = SET_ORIGIN_IMG;
    //    resetParams();
}

bool AlgorithmParams::isStarted() noexcept {
    return status != EMPTY;
}

//更新参数
void AlgorithmParams::resetParams() noexcept {
    Q_ASSERT( status != EMPTY );
    //    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( !origin_gray_image.empty() && !origin_bgr_image.empty() );
    excute_gray_image = origin_gray_image;
    excute_bgr_image  = origin_bgr_image.clone();
    Q_ASSERT( !excute_gray_image.empty() && !excute_bgr_image.empty() );
    status = SET_EXCUTE_IMG;
}

//判断参数是否合格
bool AlgorithmParams::isParamsValid() noexcept {
    const auto &[ rect_up_ptr, rect_up_binary_threshold, rect_up_mean_height, _1 ]       = rect_tuple_array.at( 0 );
    const auto &[ rect_down_ptr, rect_down_binary_threshold, rect_down_mean_height, _2 ] = rect_tuple_array.at( 1 );
    Q_UNUSED( rect_up_binary_threshold )
    Q_UNUSED( rect_down_binary_threshold )
    return rect_up_ptr && rect_down_ptr && !rect_circle_array.empty() &&
           rect_up_mean_height > 0 && rect_down_mean_height > 0 &&
           status == SET_EXCUTE_IMG;
}

void AlgorithmParams::algorithmFinished() noexcept {
    Q_ASSERT( status == SET_EXCUTE_IMG );
    status = ALGORITHM_FINISHED;
}

//读取json文件
void AlgorithmParams::getParamsFromJson() noexcept {
    static auto *algorithm_browser = AlgorithmPropertBrowserWindow::getInstance().getSubClassBrowser< AlgorithmPropertBrowser >();
    Q_ASSERT( algorithm_browser );
    algorithm_browser->setTableFromJson();
    static auto &scene = AbstractGraphicsScene::getInstance();

    auto &[ _1, rect_up_binary_threshold, rect_up_mean_height, _2 ]     = rect_tuple_array.at( 0 );
    auto &[ _3, rect_down_binary_threshold, rect_down_mean_height, _4 ] = rect_tuple_array.at( 1 );
    rect_up_binary_threshold                                            = algorithm_browser->getJsonValue< int >( 1 );
    rect_up_mean_height                                                 = algorithm_browser->getJsonValue< int >( 2 );
    rect_down_binary_threshold                                          = algorithm_browser->getJsonValue< int >( 4 );
    rect_down_mean_height                                               = algorithm_browser->getJsonValue< int >( 5 );
    //
    auto rect1_str = algorithm_browser->getJsonValue< QString >( 0 ),
         rect2_str = algorithm_browser->getJsonValue< QString >( 3 ),
         rect3_str = algorithm_browser->getJsonValue< QString >( 6 );
    if( !rect1_str.isEmpty() || !rect2_str.isEmpty() || !rect3_str.isEmpty() )
        scene.clear();
    else
        return;
    if( !rect1_str.isEmpty() ) {
        //StringOperation::str2NumArray将数字数组转换成字符串
        const auto &point_array = StringOperation::str2NumArray< int >( rect1_str );
        Q_ASSERT( point_array.size() == 4 );
        scene.addAbstractItem(
            new AbstractGraphicsItem(
                QRectF( point_array.at( 0 ), point_array.at( 1 ), point_array.at( 2 ), point_array.at( 3 ) ),
                AbstractGraphicsItem::RECT_UP ),
            AbstractGraphicsItem::RECT_UP );
    }
    if( !rect2_str.isEmpty() ) {
        const auto &point_array = StringOperation::str2NumArray< int >( rect2_str );
        Q_ASSERT( point_array.size() == 4 );
        scene.addAbstractItem(
            new AbstractGraphicsItem(
                QRectF( point_array.at( 0 ), point_array.at( 1 ), point_array.at( 2 ), point_array.at( 3 ) ),
                AbstractGraphicsItem::RECT_DOWN ),
            AbstractGraphicsItem::RECT_DOWN );
    }
    if( !rect3_str.isEmpty() ) {
        const auto &rect3_str_list = rect3_str.replace( "[", "" ).replace( "]", "" ).split( "||" );
        for( const auto &fact_rect3_str: qAsConst( rect3_str_list ) ) {
            const auto &point_array = StringOperation::str2NumArray< int >( fact_rect3_str );
            Q_ASSERT( point_array.size() == 4 );
            scene.addAbstractItem(
                new AbstractGraphicsItem(
                    QRectF( point_array.at( 0 ), point_array.at( 1 ), point_array.at( 2 ), point_array.at( 3 ) ),
                    AbstractGraphicsItem::CIRCLE ),
                AbstractGraphicsItem::CIRCLE );
        }
    }
}

//保存参数到json文件
void AlgorithmParams::storeParamsIntoJson() noexcept {
    if( status == EMPTY ) return;
    QString rect1_str, rect2_str, rect3_str;
    const auto &[ rect_up_ptr, rect_up_binary_threshold, rect_up_mean_height, _1 ]       = rect_tuple_array.at( 0 );
    const auto &[ rect_down_ptr, rect_down_binary_threshold, rect_down_mean_height, _2 ] = rect_tuple_array.at( 1 );
    if( rect_up_ptr ) {
        const auto &rect = *rect_up_ptr;
        Q_ASSERT( !rect.empty() );
        const QVector< int > &tmp = { rect.x, rect.y, rect.width, rect.height };
        rect1_str                 = StringOperation::numArray2Str( tmp );
    }
    if( rect_down_ptr ) {
        const auto &rect = *rect_down_ptr;
        Q_ASSERT( !rect.empty() );
        const QVector< int > &tmp = { rect.x, rect.y, rect.width, rect.height };
        rect2_str                 = StringOperation::numArray2Str( tmp );
    }
    if( !rect_circle_array.isEmpty() ) {
        rect3_str = "[";
        for( const auto elem: qAsConst( rect_circle_array ) ) {
            const auto &rect = *elem;
            Q_ASSERT( !rect.empty() );
            const QVector< int > &tmp      = { rect.x, rect.y, rect.width, rect.height };
            QString               rect_str = StringOperation::numArray2Str( tmp );
            rect3_str.push_back( rect_str + "||" );
        }
        rect3_str.chop( 2 );
        rect3_str += "]";
        printVar( rect3_str );
    }
    //
    static auto *algorithm_browser = AlgorithmPropertBrowserWindow::getInstance().getSubClassBrowser< AlgorithmPropertBrowser >();
    Q_ASSERT( algorithm_browser );
    algorithm_browser->flushTableValue( 0, rect1_str );
    algorithm_browser->flushTableValue( 1, rect_up_binary_threshold );
    algorithm_browser->flushTableValue( 2, rect_up_mean_height );
    algorithm_browser->flushTableValue( 3, rect2_str );
    algorithm_browser->flushTableValue( 4, rect_down_binary_threshold );
    algorithm_browser->flushTableValue( 5, rect_down_mean_height );
    algorithm_browser->flushTableValue( 6, rect3_str );
    algorithm_browser->setJsonFromTable();
    //提示保存成功
    QMessageBox::information( nullptr, u8"提示", u8"保存成功" );
}

void AlgorithmParams::resetOriginGrayImage( QImage img ) noexcept {
    //    TIMER_RECORD_FUNC_TIME;
    origin_gray_image = excute_gray_image = Cv2Qt::QImage2Mat( img ).clone();
    Q_ASSERT( !origin_gray_image.empty() );
    cv::cvtColor( origin_gray_image, origin_bgr_image, cv::COLOR_GRAY2RGB );
    excute_bgr_image = origin_bgr_image;
    status           = SET_EXCUTE_IMG;
}

void AlgorithmParams::resetOriginGrayImage( cv::Mat img ) noexcept {
    //    TIMER_RECORD_FUNC_TIME;
    origin_gray_image = excute_gray_image = img;
    Q_ASSERT( !origin_gray_image.empty() );
    cv::cvtColor( origin_gray_image, origin_bgr_image, cv::COLOR_GRAY2RGB );
    excute_bgr_image = origin_bgr_image;
    status           = SET_EXCUTE_IMG;
}
