#ifndef ICGRAPHICSITEM_H
#define ICGRAPHICSITEM_H

#include "stable.h"

class ICGraphicsItem: public QGraphicsItem {
    //    Q_OBJECT

public:
    enum {
        NONE,
        RECT_CHIP,
        RECT_LOGO,
        RECT_CIRCLE
    };
    int      rect_type = NONE;
    cv::Rect params_rect;
    ICGraphicsItem( const QRectF &rect, const int type = NONE ) noexcept;
    void               setType( int type ) noexcept;
    void               resetRect( const QRectF &rect ) noexcept;
    static const char *getEnumStr( const int type ) noexcept;
    static QColor      getColor( const int type ) noexcept;

    // QGraphicsItem interface
    virtual void   paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * ) noexcept override;
    virtual QRectF boundingRect() const noexcept override;
    virtual int    type() const noexcept override;
    cv::Rect *     getParamRect() noexcept;

protected:
    virtual void mousePressEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    virtual void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) noexcept override;

private:
    QRectF rect;

    QColor color = getColor( NONE );
};

#endif// ICGRAPHICSITEM_H
