#include "FilmStripWindow.h"
#include "AlgorithmPropertBrowserWindow.h"
#include "AlgorithmPropertBrowser.h"
#include "ui_FilmStripWindow.h"
#include "FilmStripClassifier.h"
#include "AlgorithmParams.h"
#include "CircularArrays.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <execution>
#include "ConsoleFileLogger.h"

static auto circular_array = CircularArrays< std::tuple< QString, cv::Mat, cv::Mat > >();

FilmStripWindow::FilmStripWindow( QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::FilmStripWindow )
    , algorithm_browser( AlgorithmPropertBrowserWindow::getInstance().getSubClassBrowser< AlgorithmPropertBrowser >() ) {
    ui->setupUi( this );
    setAcceptDrops( true );// 允许窗口接收拖拽事件
    ui->comboBox_ModeSelect->addItems( { "Train", "Excute" } );
    ui->comboBox_ModeSelect->setCurrentIndex( FIRST_MODE );
    connect( ui->pushButton_ReadJson, &QPushButton::clicked, this, [ this ]() {
        if( params.isStarted() ) {
            //读取json文件
            params.getParamsFromJson();
            excute();
        }
        else {
            QMessageBox::warning( nullptr, u8"警告", u8"请打开图片后再进行操作!" );
        }
    } );
    connect( ui->pushButton_StoreJson, &QPushButton::clicked, this, []() {
        if( params.isStarted() )
            //保存json文件
            params.storeParamsIntoJson();
        else
            QMessageBox::warning( nullptr, u8"警告", u8"请打开图片后再进行操作!" );
    } );
    connect( ui->action_AlgorithmConfig, &QAction::triggered, AlgorithmPropertBrowserWindow::getInstancePtr(), &QWidget::show );
    connect( ui->action_OpenImages, &QAction::triggered, this, &FilmStripWindow::open_Images_action_slot );
    ui->spinBox_Threshold1->setRange( 0, 255 );
    ui->horizontalSlider_Threshold1->setRange( 0, 255 );
    ui->spinBox_Threshold1->setValue( 70 );
    ui->horizontalSlider_Threshold1->setValue( 70 );
    connect( ui->pushButton_Run, &QPushButton::clicked, this, &FilmStripWindow::excute );
    connect( ui->spinBox_Threshold1, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, &FilmStripWindow::threshold1_changed_slot );
    connect( ui->horizontalSlider_Threshold1, static_cast< void ( QSlider::* )( int ) >( &QSlider::valueChanged ), this, [ this ]( int value ) { ui->spinBox_Threshold1->setValue( value ); } );
    connect( ui->spinBox_Threshold1, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, [ this ]( int value ) { ui->horizontalSlider_Threshold1->setValue( value ); } );
}

FilmStripWindow::~FilmStripWindow() {
    delete ui;
}

std::tuple< int, QImage > FilmStripWindow::dealWithQImage( QImage gray_img ) noexcept {
    NoteTimingStart;
    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( !gray_img.isNull() );
    params.resetOriginGrayImage( gray_img );
    if( !params.isParamsValid() )
        params.getParamsFromJson();
    if( params.isParamsValid() ) {
        const int result = FilmStripClassifier().excute();
        Q_ASSERT( params.status == AlgorithmParams::ALGORITHM_FINISHED );
        return { result, Cv2Qt::Mat2QImage( params.excute_bgr_image ) };
    }
    else
        return { -1, {} };
}

std::tuple< int, QImage > FilmStripWindow::dealWithMat( cv::Mat gray_img ) noexcept {
    //    TIMER_RECORD_FUNC_TIME;
    Q_ASSERT( !gray_img.empty() );
    params.resetOriginGrayImage( gray_img );
    if( !params.isParamsValid() )
        params.getParamsFromJson();
    if( params.isParamsValid() ) {
        const int result = FilmStripClassifier().excute();
        Q_ASSERT( params.status == AlgorithmParams::ALGORITHM_FINISHED );
        return { result, Cv2Qt::Mat2QImage( params.excute_bgr_image ) };
    }
    else
        return { -1, {} };
}

void FilmStripWindow::dragEnterEvent( QDragEnterEvent *event ) noexcept {
    //接受拖拽事件
    event->accept();
    // 有拖拽文件时设置接受
    if( event->mimeData()->hasUrls() ) {
        event->acceptProposedAction();
    }
    QMainWindow::dragEnterEvent( event );
}

//用户拖拽文件到窗口时，会触发这个事件
void FilmStripWindow::dropEvent( QDropEvent *event ) noexcept {
    // mimeData() 函数获取到拖拽事件中传输的数据
    //event->mimeData()->urls() 返回一个 QList<QUrl>，表示拖拽事件中拖拽的文件路径。
    const auto &  urls = event->mimeData()->urls();
    QFileInfoList file_info_list;
    //reserve分配空间
    //文件的路径保存在QFileInfoList中
    file_info_list.reserve( urls.size() );
    //遍历所有路径（包括子目录里面的路径）
    for( const auto &url: urls ) {
        const auto &info = QFileInfo( url.toLocalFile() );
        if( info.isFile() && isFilePathValid( info.absoluteFilePath() ) )
            file_info_list << info;//如果是文件且文件名符合规定
        else if( info.isDir() )
            file_info_list += dealWithDirPath( info );//如果路径是子目录，获取该子目录下的所有文件
    }
    if( file_info_list.empty() ) return;
    //文件路径保存在img_path_list
    QStringList img_path_list;
    img_path_list.reserve( file_info_list.size() );
    for( const auto &file_info: qAsConst( file_info_list ) ) {
        img_path_list << file_info.absoluteFilePath();
    }
    dropEvent_Help( std::move( img_path_list ) );
    /*QMainWindow::dropEvent( event )是调用基类QMainWindow的dropEvent函数，确保该事件被正确处理。
     * 因为FilmStripWindow是QMainWindow的派生类，如果在处理完拖拽事件之后不调用基类的dropEvent函数，
     * 可能会导致一些意外的问题，例如窗口不能获得焦点或者窗口关闭按钮不起作用等。*/
    QMainWindow::dropEvent( event );
}

void FilmStripWindow::threshold1_changed_slot( int value ) noexcept {
    if( params.status == AlgorithmParams::EMPTY || !params.rect_binary_threshold_ptr )
        return;
    *params.rect_binary_threshold_ptr = value;
    excute();
}

void FilmStripWindow::on_comboBox_ModeSelect_currentIndexChanged( int index ) noexcept {
    Q_ASSERT( index >= 0 && index < 2 );
    mode = index;
    if( params.status == AlgorithmParams::EMPTY )
        return;
    else
        excute();
}

void FilmStripWindow::open_Images_action_slot() noexcept {
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


void FilmStripWindow::dropEvent_Help( QStringList path_list ) noexcept {
    if( path_list.empty() ) return;
    circular_array.clear();
    //根据传入的文件路径列表读取图片，并保存到一个循环数组中
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
    //更新图片
    params.resetOriginImage( gray, bgr );
    //qobject_cast将对象 ui->paintingView指针转换为另一个类AbstractPaintingView的指针
    //断言 ui->paintingView 是否是 AbstractPaintingView 类型，如果不是则会触发断言失败并中止程序运行
    Q_ASSERT( qobject_cast< AbstractPaintingView * >( ui->paintingView ) );
    //将 bgr 作为参数设置到 ui->paintingView中
    qobject_cast< AbstractPaintingView * >( ui->paintingView )->setImage( bgr );
    //    excute();
}


void FilmStripWindow::excute() noexcept {
    if( params.status == AlgorithmParams::EMPTY ) {
        QMessageBox::warning( nullptr, u8"警告", u8"请打开图片后再进行操作!" );
        return;
    }
    switch( mode ) {
        case TRAIN:
            FilmStripClassifier::train();
            break;
        case EXCUTE:
            params.resetParams();
            if( params.isParamsValid() ) {
                FilmStripClassifier().excute();
                Q_ASSERT( params.status == AlgorithmParams::ALGORITHM_FINISHED );
            }
            else {
                auto &&str = params.paramRectStatus();
                Q_ASSERT( !str.isEmpty() );
                QMessageBox::warning( nullptr, u8"警告", QString( u8"算法无法执行!\n"
                                                                  "因为:缺少 %1 !\n"
                                                                  "请\"返回训练模式继续标注\"或者\"读取json数据\"!" )
                                                             .arg( str ) );
            }
            break;
    }
    Q_ASSERT( qobject_cast< AbstractPaintingView * >( ui->paintingView ) );
    qobject_cast< AbstractPaintingView * >( ui->paintingView )->setImage( params.excute_bgr_image );
}

bool FilmStripWindow::isFilePathValid( const QString &file_path ) noexcept {
    if( QFileInfo( file_path ).isFile() && file_path.contains( ".bmp" ) && !file_path.contains( "-result.bmp" ) )
        return true;
    else
        return false;
}

QFileInfoList FilmStripWindow::dealWithDirPath( const QFileInfo &info ) noexcept {
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

void FilmStripWindow::keyPressEvent( QKeyEvent *event ) noexcept {
    QWidget::keyPressEvent( event );
    if( event->key() == Qt::Key_Tab && event->modifiers() == Qt::CTRL && circular_array.size() > 1 ) {
        auto &[ title, gray, bgr ] = circular_array.getNext();
        params.resetOriginImage( gray, bgr );
        excute();
        setWindowTitle( title );
    }
}

void FilmStripWindow::closeEvent(QCloseEvent *) noexcept
{
//    if (params.isParamsValid())
//    {
//        emit sendMsg("heat1");
//        emit writeLog("heat1", 0);
//    }
//    else
//    {
//        emit sendMsg("heat0");
//        emit writeLog("heat0", 1);
//    }

    //暂时返回成功先
    emit sendMsg("WakeUp heat 1");
//    emit writeLog("WakeUp heat 1", 0);
}
