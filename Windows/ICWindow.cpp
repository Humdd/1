#include "ICWindow.h"
#include "ICGraphicsItem.h"
#include "ICAlgorithmPropertBrowserWindow.h"
#include "ICAlgorithmPropertBrowser.h"
#include "ui_ICWindow.h"
#include "ICClassifier.h"
#include "ICAlgorithmParams.h"
#include "ICCircularArrays.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <execution>
#include "ConsoleFileLogger.h"

static auto circular_array = CircularArrays< std::tuple< QString, cv::Mat, cv::Mat > >();

//指定icwindow父窗口为Qmainwindow
//algorithm_browser实现主窗口中创建一个用于显示算法属性的浏览器窗口
ICWindow::ICWindow( QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::ICWindow )
    , algorithm_browser( ICAlgorithmPropertBrowserWindow::getInstance().getSubClassBrowser< ICAlgorithmPropertBrowser >() ) {
    ui->setupUi( this );
    setAcceptDrops( true );// 允许窗口接收拖拽事件
    ui->comboBox_ModeSelect->addItems( { "Train", "Excute" } );
    ui->comboBox_ModeSelect->setCurrentIndex( FIRST_MODE );//初始是Train模式
    //读取json文件获取参数
    connect( ui->pushButton_ReadJson, &QPushButton::clicked, this, [ this ]() {
        //lambda表达式捕获this，拥有和当前类成员同样的访问权限
        //params_ic是ICAlgorithmParams参数类
        if( params_ic.isStarted() ) {
            params_ic.getParamsFromJson();
            execute();
        }
        else {
            QMessageBox::warning( nullptr, u8"警告", u8"请打开图片后再进行操作!" );
        }
    } );
    //保存json参数
    connect( ui->pushButton_StoreJson, &QPushButton::clicked, this, []() {
        if( params_ic.isStarted() )
            params_ic.storeParamsIntoJson();
        else
            QMessageBox::warning( nullptr, u8"警告", u8"请打开图片后再进行操作!" );
    } );
    connect( ui->action_AlgorithmConfig, &QAction::triggered, ICAlgorithmPropertBrowserWindow::getInstancePtr(), &QWidget::show );
    connect( ui->action_OpenImages, &QAction::triggered, this, &ICWindow::open_Images_action_slot );
    ui->spinBox_Threshold1->setRange( 0, 255 );
    ui->horizontalSlider_Threshold1->setRange( 0, 255 );
    ui->spinBox_Threshold1->setValue( 70 );
    ui->horizontalSlider_Threshold1->setValue( 70 );
    connect( ui->pushButton_Run, &QPushButton::clicked, this, &ICWindow::execute );
    connect( ui->spinBox_Threshold1, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, &ICWindow::threshold1_changed_slot );
    connect( ui->horizontalSlider_Threshold1, static_cast< void ( QSlider::* )( int ) >( &QSlider::valueChanged ), this, [ this ]( int value ) { ui->spinBox_Threshold1->setValue( value ); } );
    connect( ui->spinBox_Threshold1, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, [ this ]( int value ) { ui->horizontalSlider_Threshold1->setValue( value ); } );
}

ICWindow::~ICWindow() {
    delete ui;
}

std::tuple< int, QImage > ICWindow::dealWithQImage( QImage gray_img ) noexcept {
    NoteTimingStart;
    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( !gray_img.isNull() );
    params_ic.resetOriginGrayImage( gray_img );
    if( !params_ic.isParamsValid() )
        params_ic.getParamsFromJson();
    if( params_ic.isParamsValid() ) {
        const int result = ICClassifier().execute();
        Q_ASSERT( params_ic.status == ICAlgorithmParams::ALGORITHM_FINISHED );
        return { result, Cv2Qt::Mat2QImage( params_ic.excute_bgr_image ).copy() };
    }
    else
        return { -1, {} };
}

std::tuple< int, QImage > ICWindow::dealWithMat( cv::Mat gray_img ) noexcept {
    //    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( !gray_img.empty() );
    params_ic.resetOriginGrayImage( gray_img );
    if( !params_ic.isParamsValid() )
        params_ic.getParamsFromJson();
    if( params_ic.isParamsValid() ) {
        const int result = ICClassifier().execute();
        Q_ASSERT( params_ic.status == AlgorithmParams::ALGORITHM_FINISHED );
        return { result, Cv2Qt::Mat2QImage( params_ic.excute_bgr_image ).copy() };
    }
    else
        return { -1, {} };
}

void ICWindow::dragEnterEvent( QDragEnterEvent *event ) noexcept {
    //接受拖拽事件
    event->accept();
    // 有拖拽文件时设置接受
    if( event->mimeData()->hasUrls() ) {
        event->acceptProposedAction();
    }
    QMainWindow::dragEnterEvent( event );
}

void ICWindow::dropEvent( QDropEvent *event ) noexcept {
    const auto &  urls = event->mimeData()->urls();
    QFileInfoList file_info_list;
    file_info_list.reserve( urls.size() );
    //遍历所有路径（包括子目录里面的路径）
    for( const auto &url: urls ) {
        const auto &info = QFileInfo( url.toLocalFile() );
        if( info.isFile() && isFilePathValid( info.absoluteFilePath() ) )
            file_info_list << info;//如果是文件且文件名符合规定
        else if( info.isDir() )
            file_info_list += dealWithDirPath( info );//如果路径是子目录，继续递归遍历子目录
    }
    if( file_info_list.empty() ) return;
    QStringList img_path_list;
    img_path_list.reserve( file_info_list.size() );
    for( const auto &file_info: qAsConst( file_info_list ) ) {
        img_path_list << file_info.absoluteFilePath();
    }
    dropEvent_Help( std::move( img_path_list ) );
    //防止父窗口出现异常
    QMainWindow::dropEvent( event );
}

void ICWindow::threshold1_changed_slot( int value ) noexcept {
    if( params_ic.status == ICAlgorithmParams::EMPTY || !params_ic.rect_binary_threshold_ptr )
        return;
    *params_ic.rect_binary_threshold_ptr = value;
//    printVar(*params_ic.rect_binary_threshold_ptr);
    execute();
}

void ICWindow::on_comboBox_ModeSelect_currentIndexChanged( int index ) noexcept {
    Q_ASSERT( index >= 0 && index < 2 );
    mode = index;
    if( params_ic.status == ICAlgorithmParams::EMPTY )
        return;
    else
        execute();
}

//批量打开图片
void ICWindow::open_Images_action_slot() noexcept {
    auto &&img_path_list = QFileDialog::getOpenFileNames( this, QStringLiteral( "芯片图片选择框！" ), "", QStringLiteral( "芯片图片(*bmp)" ) );
    if( img_path_list.isEmpty() )
        return;
    for( int index = 0; index < img_path_list.size(); ++index ) {
        if( !isFilePathValid( img_path_list[ index ] ) ) {
            img_path_list.removeAt( index );
            --index;
        }
    }
    dropEvent_Help( std::move( img_path_list ) );
}

//拖拽图片进窗口
void ICWindow::dropEvent_Help( QStringList path_list ) noexcept {
    if( path_list.empty() ) return;
    circular_array.clear();
    for( const auto &img_path: path_list ) {
        cv::Mat origin_bgr_image, origin_gray_image;
        origin_bgr_image = cv::imread( Cv2Qt::validImagePath( img_path ), cv::IMREAD_COLOR );
        Q_ASSERT( !origin_bgr_image.empty() );
        cv::cvtColor( origin_bgr_image, origin_gray_image, cv::COLOR_RGB2GRAY );
        Q_ASSERT( !origin_gray_image.empty() );
        circular_array.emplace( std::move( img_path ), std::move( origin_gray_image ), std::move( origin_bgr_image ) );
    }
    Q_ASSERT( !circular_array.empty() );
    auto &[ title, gray, bgr ] = circular_array.first();
    setWindowTitle( title );
    params_ic.resetOriginImage( gray, bgr );
    Q_ASSERT( qobject_cast< ICPaintingView * >( ui->paintingView ) );
    //将父窗口的画布转换成子窗口的画布
    qobject_cast< ICPaintingView * >( ui->paintingView )->setImage( bgr );
    //    excute();
}

void ICWindow::execute() noexcept {
    if( params_ic.status == ICAlgorithmParams::EMPTY ) {
        QMessageBox::warning( nullptr, u8"警告", u8"请打开图片后再进行操作!" );
        return;
    }
    switch( mode ) {
        case TRAIN:
            ICClassifier::train();
            break;
        case EXCUTE:
            params_ic.resetParams();
            if( params_ic.isParamsValid() ) {
                const int result = ICClassifier().execute();
                Q_ASSERT( params_ic.status == ICAlgorithmParams::ALGORITHM_FINISHED );
            }
            else {
                auto &&str = params_ic.paramRectStatus();
                Q_ASSERT( !str.isEmpty() );
                QMessageBox::warning( nullptr, u8"警告", QString( u8"算法无法执行!\n"
                                                                  "因为:缺少 %1 !\n"
                                                                  "请\"返回训练模式继续标注\"或者\"读取json数据\"!" )
                                                             .arg( str ) );
            }
            break;
    }
    Q_ASSERT( qobject_cast< ICPaintingView * >( ui->paintingView ) );
    qobject_cast< ICPaintingView * >( ui->paintingView )->setImage( params_ic.excute_bgr_image );
}

bool ICWindow::isFilePathValid( const QString &file_path ) noexcept {
    if( QFileInfo( file_path ).isFile() && file_path.contains( ".bmp" ) && !file_path.contains( "-result.bmp" ) )
        return true;
    else
        return false;
}

QFileInfoList ICWindow::dealWithDirPath( const QFileInfo &info ) noexcept {
    const auto &  file_infos = FileManager::getAllFilePath( info.absoluteFilePath(), { "*.bmp" } );
    QFileInfoList dst;
    dst.reserve( file_infos.size() );
    for( const auto &elem: file_infos ) {
        if( elem.isFile() && isFilePathValid( elem.absoluteFilePath() ) )
            dst.push_back( elem );
        else if( elem.isDir() )
            dst += dealWithDirPath( elem );
    }
    return dst;
}

void ICWindow::keyPressEvent( QKeyEvent *event ) noexcept {
    QWidget::keyPressEvent( event );
    if( event->key() == Qt::Key_Tab && event->modifiers() == Qt::CTRL && circular_array.size() > 1 ) {
        auto &[ title, gray, bgr ] = circular_array.getNext();
        params_ic.resetOriginImage( gray, bgr );
        execute();
        setWindowTitle( title );
    }
}

void ICWindow::closeEvent(QCloseEvent *) noexcept
{
//    if (params_ic.isParamsValid())
//    {
//        emit sendMsg("ic1");
//        emit writeLog("ic1", 0);
//    }
//    else
//    {
//        emit sendMsg("ic0");
//        emit writeLog("ic0", 1);
//    }

    //暂时返回成功先
    emit sendMsg("WakeUp ic 1");
//    emit writeLog("WakeUp ic 1", 0);

}

