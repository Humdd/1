#include "AbstractPaintingView.h"
#include "ui_AbstractPaintingView.h"
#include <QKeyEvent>
#include <QMimeData>

AbstractPaintingView::AbstractPaintingView( QWidget *parent )
    : QWidget( parent )
    , ui( new Ui::AbstractPaintingView ) {
    ui->setupUi( this );
    // 设置窗口无边框
    setWindowFlag( Qt::FramelessWindowHint );
    // 允许窗口接收拖拽事件
    setAcceptDrops( true );
    ui->graphicsView->setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    ui->graphicsView->setResizeAnchor( QGraphicsView::AnchorUnderMouse );
    ui->graphicsView->setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
}

AbstractPaintingView::~AbstractPaintingView() {
    delete ui;
}

QGraphicsView *AbstractPaintingView::getGraphicsView() {
    return ui->graphicsView;
}

QGraphicsScene *AbstractPaintingView::getGraphicsScene() {
    return ui->graphicsView->scene();
}

void AbstractPaintingView::displayFirstPixmap() {
    current_index         = 0;
    auto &[ _, _pixmap ] = img_info_array[ current_index ];
    pixmap.setPixmap( _pixmap->pixmap() );
    viewFitPixmapSize();
}

void AbstractPaintingView::dropEventHelper( const QStringList &img_path_list ) {
    if( img_path_list.empty() ) return;
    //    SPDLOG_TRACE_FUNC();

    QFileInfoList file_infos;
    file_infos.reserve( img_path_list.size() );

    //遍历所有路径（包括子目录里面的路径）
    for( const auto &path: img_path_list ) {
        const auto &info = QFileInfo( path );
        if( info.isFile() && isFilePathValid( info ) )
            file_infos << info;//如果是文件且文件名符合规定
        else if( info.isDir() )
            file_infos += dealWithDirPath( path );//如果路径是子目录，继续递归遍历子目录
    }

    if( file_infos.empty() ) return;
    img_info_array.clear();//清空
    img_info_array.reserve( file_infos.size() );
    for( const auto &elem: file_infos ) {//对于每一张图片，保存Pixmap与部分文件名
        img_info_array.emplace_back( elem.filePath(), new QGraphicsPixmapItem( elem.filePath() ) );
    }
    Q_ASSERT( !img_info_array.empty() );
    displayFirstPixmap();//显示第一张图片
}

QGraphicsScene *AbstractPaintingView::getScene() noexcept {
    Q_ASSERT( ui->graphicsView->scene() );
    return ui->graphicsView->scene();
}
//------------------------------------------------------------
/*!
 *@brief   Init Scene
 *@param   scene         [QGraphicsScene*]         要放入的Scene
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::initScene( QGraphicsScene *scene ) {
    Q_ASSERT( scene != nullptr );
    //设置scene
    ui->graphicsView->setScene( scene );
    //设置要对基元的边缘进行抗锯齿处理
    ui->graphicsView->setRenderHint( QPainter::Antialiasing );
    //设置显示的图像放在视图中央
    ui->graphicsView->setTransformationAnchor( QGraphicsView::AnchorViewCenter );
    scene->addItem( &pixmap );
    //QGraphicsView不接受拖放事件
    ui->graphicsView->setAcceptDrops( false );
}
//------------------------------------------------------------
/*!
 *@brief   reset display image（use the pixmap of StepWindowsInfo object）
 *@param   void
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::resetPixmap() {
    RELEASE_ASSERT( current_index >= 0 && current_index < img_info_array.size() );
    pixmap.setPixmap( std::get< 1 >( img_info_array[ current_index ] )->pixmap() );
    viewFitPixmapSize();//根据显示的图像重新调整视图尺寸
}
//------------------------------------------------------------
/*!
 *@brief   根据显示的图像重新调整视图尺寸
 *@param   void
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::viewFitPixmapSize() {
    ui->graphicsView->setSceneRect( pixmap.boundingRect() );
    ui->graphicsView->scene()->setSceneRect( pixmap.boundingRect() );
    getScene()->update();
}

void AbstractPaintingView::keyPressEvent( QKeyEvent *event ) {
    QWidget::keyPressEvent( event );
    if( event->key() == Qt::Key_Tab && event->modifiers() == Qt::CTRL ) {
        Q_ASSERT( current_index >= 0 && current_index < img_info_array.size() );
        current_index = ( current_index + 1 ) % img_info_array.size();
        resetPixmap();
    }
}

bool AbstractPaintingView::isFilePathValid( const QFileInfo &info ) const {
    return info.absoluteFilePath().contains( ".bmp" );
}

QFileInfoList AbstractPaintingView::dealWithDirPath( const QFileInfo &info ) const {
    const auto &  file_infos = FileManager::getAllFilePath( info.absoluteFilePath(), { "*.bmp" } );
    QFileInfoList dst;

    for( const auto &elem: file_infos ) {
        if( elem.isFile() && isFilePathValid( elem ) ) {
            dst.push_back( elem );
        }
        else if( elem.isDir() ) {
            dst += dealWithDirPath( elem );
        }
    }

    return dst;
}

void AbstractPaintingView::wheelEvent( QWheelEvent *event ) {
    //按下Ctrl时才用滚轮来缩放图像
    if( event->modifiers() == Qt::CTRL ) {
        scaleView( pow( 2, event->angleDelta().y() / 240.0 ) );
    }
}

void AbstractPaintingView::dragEnterEvent( QDragEnterEvent *event ) {
    //接受拖拽事件
    event->accept();

    // 有拖拽文件时设置接受
    if( event->mimeData()->hasUrls() ) {
        event->acceptProposedAction();
    }

    QWidget::dragEnterEvent( event );
}
//------------------------------------------------------------
/*!
 *@brief   deal with drop event
 *@param   event         [QDropEvent*]         要被处理的拖拽东西的信息
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::dropEvent( QDropEvent *event ) {
    const auto &urls = event->mimeData()->urls();
    QStringList img_path_list;
    img_path_list.reserve( urls.size() );
    for( const auto &elem: urls ) {
        img_path_list << elem.toLocalFile();
    }
    dropEventHelper( img_path_list );
    QWidget::dropEvent( event );
}
//------------------------------------------------------------
/*!
 *@brief   scale the View factly
 *@param   scaleFactor         [qreal]         视图缩放的倍数
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::scaleView( qreal scaleFactor ) {
    ui->graphicsView->scale( scaleFactor, scaleFactor );
}
