#ifndef ABSTRACTGRAPHICSSCENE_H
#define ABSTRACTGRAPHICSSCENE_H
#include "AbstractGraphicsItem.h"

class AbstractGraphicsScene
    : public QGraphicsScene
    , public SingletonTemplate< AbstractGraphicsScene > {
    Q_OBJECT

public:
    static void removeItemRectType( AbstractGraphicsItem *item ) noexcept;
    static void resetItemRectType( AbstractGraphicsItem *item, const int type ) noexcept;
    void        removeAbstractItem( AbstractGraphicsItem *item ) noexcept;
    void        addAbstractItem( AbstractGraphicsItem *item, const int type = AbstractGraphicsItem::CIRCLE ) noexcept;
    void        resetItemRectType( const int type, AbstractGraphicsItem *item ) noexcept;
    void        clear() noexcept;

    // QGraphicsScene interface

protected:
    virtual void          mousePressEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    virtual void          mouseMoveEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    virtual void          mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) noexcept override;
    QPointF               start_point, end_point;
    AbstractGraphicsItem *item = nullptr;

private:
    std::vector< std::unique_ptr< AbstractGraphicsItem > > item_array;
    explicit AbstractGraphicsScene( QObject *parent = nullptr ) noexcept;
    Q_DECLARE_SINGLETON( AbstractGraphicsScene )
};
#endif// ABSTRACTGRAPHICSSCENE_H
