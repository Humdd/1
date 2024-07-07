#ifndef DISPLAYIMAGEWIDGET_H
#define DISPLAYIMAGEWIDGET_H
#include <QWidget>
#include <QThread>
#include <QMutex>
class DisplayImageWidget : public QWidget {
    Q_OBJECT
public:
    explicit DisplayImageWidget( QWidget *parent = nullptr );
    void resetImage( const QImage &image );
    void resetImage( const QPixmap &pixmap );
    void resetAndCopyImage( const QImage &image );

protected:
    virtual void paintEvent( QPaintEvent *event ) override;

private:
    QMutex mutex;
    QPixmap pixmap;
    QImage image;
    QRect fact_rect = {1595, 820, 830, 1245} ; //1595

};

#endif // DISPLAYIMAGEWIDGET_H
