#include "DisplayImageWidget.h"
#include <QPainter>
#include "ConsoleFileLogger.h"
#include <atomic>

DisplayImageWidget::DisplayImageWidget( QWidget *parent )
    : QWidget( parent ) {
}

//更新界面上显示的图像
void DisplayImageWidget::resetImage( const QImage &image ) {
    assert( image.isNull() == false );
    //方案一
//    pixmap = QPixmap::fromImage(image);  //消耗cpu；使用QImage避免了转换过程，节省了资源。
    //方案二
    mutex.lock();//使用互斥量来保证线程安全，避免多个线程同时访问 image 变量而导致数据竞争。
    this->image = image;
    mutex.unlock();
    update();
}

void DisplayImageWidget::resetImage( const QPixmap &pixmap ) {
    assert( image.isNull() == false );
    //方案一
//        pixmap = QPixmap::fromImage(image);  //耗cpu
    //方案二
    this->pixmap = pixmap;
    update();
}

void DisplayImageWidget::resetAndCopyImage( const QImage &image ) {
    resetImage( image.copy() );
}
extern std::atomic_bool needRepaint;
void DisplayImageWidget::paintEvent( QPaintEvent * ) {
    if( image.isNull() ) {
        return;
    }
    QPainter painter( this );
    //    pixmap.scaled(pixmap.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //方案一
//        painter.drawPixmap( QRect( 0, 0, width(), height() ), pixmap );
    //方案二

    int min_index = 0;
    if( width() / 4024.0 >= height() / 3036.0 ) {
        min_index = 1;
    }
    const double min_size = min_index == 0 ? width() : height();
    QRectF rect2;
    if( min_index == 0 ) {
        rect2 = QRectF{
                0,0,
            min_size,
            min_size * ( 3036.0/4024.0 )
        };
    }
    else {
        rect2 = QRectF{
                0,0,
            min_size * ( 4024.0/3036.0 ),
            min_size
        };
    }
    double detaX = ( rect().width() - rect2.width() ) / 2,
           detaY = ( rect().height() - rect2.height() ) / 2;
    rect2 = QRectF{
        rect2.x() + detaX,
        rect2.y() + detaY,
        rect2.width(),
        rect2.height()
    };
    mutex.lock();
    //显示图片
    painter.drawImage( rect2, image );
//    painter.drawPixmap(rect2.toRect(), pixmap);

    // 画红色矩形框
//    QPen pen;
//    pen.setWidth(4);
//    pen.setColor(QColor(Qt::red));
//    painter.setPen(pen);
//    double ratio_x = (double)rect2.width() / 4024.0,
//           ratio_y = (double)rect2.height() / 3036.0;
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.drawRect(fact_rect.x() * ratio_x + detaX, fact_rect.y() * ratio_y + detaY,
//                     fact_rect.width() * ratio_x, fact_rect.height() * ratio_y);
    mutex.unlock();
}


