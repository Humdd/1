#ifndef SYNCHRONIZEDVIEW_H
#define SYNCHRONIZEDVIEW_H
#include "stable.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QWidget>

namespace Ui {
    class SynchronizedView;
}

class SynchronizedView: public QWidget {
    Q_OBJECT

public:
    explicit SynchronizedView( QWidget *parent = nullptr ) noexcept;
    ~SynchronizedView();
    void setImage( const cv::Mat &_image ) noexcept;

protected:
    // QWidget interface
    virtual void          wheelEvent( QWheelEvent *event ) noexcept override;
    void                  scaleView( qreal scaleFactor ) noexcept;
    Ui::SynchronizedView *ui;
    QGraphicsPixmapItem   display_pixmap;
};

#endif// SYNCHRONIZEDVIEW_H
