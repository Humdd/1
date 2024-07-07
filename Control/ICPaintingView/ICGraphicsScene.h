#ifndef ICGRAPHICSSCENE_H
#define ICGRAPHICSSCENE_H
#include "ICGraphicsItem.h"

class ICGraphicsScene
    : public QGraphicsScene
    , public SingletonTemplate< ICGraphicsScene > {
    Q_OBJECT

public:
    static void removeItemRectType( ICGraphicsItem *item ) noexcept;
    static void resetItemRectType( ICGraphicsItem *item, const int type ) noexcept;
    void        removeAbstractItem( ICGraphicsItem *item ) noexcept;
    void        addAbstractItem( ICGraphicsItem *item, const int type ) noexcept;
    void        resetItemRectType( const int type, ICGraphicsItem *item ) noexcept;
    void        clear() noexcept;

    // QGraphicsScene interface

protected:
    virtual void    mousePressEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    virtual void    mouseMoveEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    virtual void    mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    QPointF         start_point, end_point;
    ICGraphicsItem *item = nullptr;

private:
    std::vector< std::unique_ptr< ICGraphicsItem > > item_array;
    explicit ICGraphicsScene( QObject *parent = nullptr ) noexcept;
    Q_DECLARE_SINGLETON( ICGraphicsScene )
};
#endif// ICGRAPHICSSCENE_H
