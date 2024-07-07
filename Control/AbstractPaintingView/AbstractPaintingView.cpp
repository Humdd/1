#include "AbstractPaintingView.h"
#include "ui_AbstractPaintingView.h"
#include "AbstractGraphicsScene.h"
#include <QWheelEvent>

//热合相机ui
AbstractPaintingView::AbstractPaintingView( QWidget *parent ) noexcept
    : QWidget( parent )
    , ui( new Ui::AbstractPaintingView ) {
    ui->setupUi( this );
    initScene( AbstractGraphicsScene::getInstancePtr() );
    // 设置窗口无边框
    setWindowFlag( Qt::FramelessWindowHint );
    //    ui->graphicsView->setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    ui->graphicsView->setResizeAnchor( QGraphicsView::AnchorUnderMouse );
    ui->graphicsView->setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
}

AbstractPaintingView::~AbstractPaintingView() {
    delete ui;
}

AbstractGraphicsScene *AbstractPaintingView::getScene() const noexcept {
    Q_ASSERT( qobject_cast< AbstractGraphicsScene * >( ui->graphicsView->scene() ) );
    return qobject_cast< AbstractGraphicsScene * >( ui->graphicsView->scene() );
}

QGraphicsView *AbstractPaintingView::getView() const noexcept {
    return ui->graphicsView;
}

//------------------------------------------------------------
/*!
 *@brief   Init Scene
 *@param   scene         [QGraphicsScene*]         要放入的Scene
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::initScene( QGraphicsScene *scene ) noexcept {
    Q_ASSERT( scene );
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

void AbstractPaintingView::setImage( const cv::Mat &image ) noexcept {
    Q_ASSERT( !image.empty() );
    pixmap.setPixmap( Cv2Qt::Mat2Pixmap( image ) );
    viewFitPixmapSize();//根据显示的图像重新调整视图尺寸
}
//------------------------------------------------------------
/*!
 *@brief   根据显示的图像重新调整视图尺寸
 *@param   void
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::viewFitPixmapSize() noexcept {
    ui->graphicsView->setSceneRect( pixmap.boundingRect() );
    ui->graphicsView->scene()->setSceneRect( pixmap.boundingRect() );
    getScene()->update();
}

bool AbstractPaintingView::isFilePathValid( const QFileInfo &info ) const noexcept {
    return info.absoluteFilePath().contains( ".bmp" );
}

void AbstractPaintingView::wheelEvent( QWheelEvent *event ) noexcept {
    //按下Ctrl时才用滚轮来缩放图像
    if( event->modifiers() == Qt::CTRL ) {
        scaleView( pow( 2, event->angleDelta().y() / 240.0 ) );
    }
}

//------------------------------------------------------------
/*!
 *@brief   scale the View factly
 *@param   scaleFactor         [qreal]         视图缩放的倍数
 *@return  void
 */
//------------------------------------------------------------
void AbstractPaintingView::scaleView( qreal scaleFactor ) noexcept {
    ui->graphicsView->scale( scaleFactor, scaleFactor );
}
