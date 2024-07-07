#include "SynchronizedView.h"
#include "ui_SynchronizedView.h"

SynchronizedView::SynchronizedView( QWidget *parent ) noexcept
    : QWidget( parent )
    , ui( new Ui::SynchronizedView ) {
    ui->setupUi( this );
    setWindowFlags( Qt::FramelessWindowHint );
    QGraphicsScene *scene = new QGraphicsScene( this );
    ui->graphicsView->setScene( scene );
    ui->graphicsView->setRenderHint( QPainter::Antialiasing );
    ui->graphicsView->setTransformationAnchor( QGraphicsView::AnchorViewCenter );
    scene->addItem( &display_pixmap );
}

SynchronizedView::~SynchronizedView() {
    delete ui;
}

void SynchronizedView::setImage( const cv::Mat &_image ) noexcept {
    assert( !_image.empty() );
    display_pixmap.setPixmap( Cv2Qt::Mat2Pixmap( _image ) );
}

void SynchronizedView::wheelEvent( QWheelEvent *event ) noexcept {
    if( isCtrlPressed ) {
        scaleView( pow( 2, event->angleDelta().y() / 240.0 ) );
    }
}

void SynchronizedView::scaleView( qreal scaleFactor ) noexcept {
    //    qreal factor = ui->graphicsView->transform()
    //                   .scale( scaleFactor, scaleFactor )
    //                   .mapRect( QRectF( 0, 0, 1, 1 ) ).width();
    //    if ( factor < 0.07 || factor > 7 ) {
    //        return;
    //    }
    ui->graphicsView->scale( scaleFactor, scaleFactor );
    ui->graphicsView_2->scale( scaleFactor, scaleFactor );
}
