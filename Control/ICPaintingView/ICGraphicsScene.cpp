#include "ICGraphicsScene.h"
#include "ICGraphicsItem.h"
#include "ICAlgorithmParams.h"
#include <QGraphicsSceneMouseEvent>

ICGraphicsScene::ICGraphicsScene( QObject *parent ) noexcept
    : QGraphicsScene{ parent } {
    item_array.reserve( 8 );
}

void ICGraphicsScene::removeItemRectType( ICGraphicsItem *item ) noexcept {
    Q_ASSERT( item );
    Q_ASSERT( item->rect_type >= 0 && item->rect_type < 4 );
    resetItemRectType( item, ICGraphicsItem::NONE );
}

void ICGraphicsScene::resetItemRectType( ICGraphicsItem *item, const int type ) noexcept {
    Q_ASSERT( item );
    Q_ASSERT( type >= 0 && type < 4 );
#ifdef _DEBUG
    if( item->rect_type == ICGraphicsItem::RECT_CIRCLE ) {
        printVar( item->getParamRect() );
        printVar( std::get< 0 >( params_ic.rect_tuple_array.back() ) );
        if( std::get< 0 >( params_ic.rect_tuple_array.back() ) )
            printVar( *std::get< 0 >( params_ic.rect_tuple_array.back() ) );
    }
#endif
    for( auto &[ rect_ptr, _1, _2, _3 ]: params_ic.rect_tuple_array ) {
        if( rect_ptr == item->getParamRect() ) {
            rect_ptr = nullptr;
            break;
        }
    }
    if( type == ICGraphicsItem::NONE )
        return;
    std::get< 0 >( params_ic.rect_tuple_array[ type - 1 ] ) = item->getParamRect();
}

void ICGraphicsScene::removeAbstractItem( ICGraphicsItem *item ) noexcept {
    Q_ASSERT( items().contains( item ) );
    removeItemRectType( item );
    //查找item所处位置的迭代器 因为已经断言items().contains( item )， 所以iter必定有效
    auto iter = std::find_if( item_array.begin(), item_array.end(), [ item ]( auto &&ptr ) {
        return ptr.get() == item;
    } );
    Q_ASSERT( iter != item_array.end() );
    item_array.erase( iter );//数组删除item，智能指针能让item自动析构,item析构会自己从scene中删除
}

void ICGraphicsScene::addAbstractItem( ICGraphicsItem *item, const int type ) noexcept {
    Q_ASSERT( !items().contains( item ) );
    item_array.emplace_back( item );
    resetItemRectType( item, type );
    QGraphicsScene::addItem( item );
}

void ICGraphicsScene::resetItemRectType( const int type, ICGraphicsItem *item ) noexcept {
    Q_ASSERT( type >= 0 && type < 4 );
    Q_ASSERT( item );
    Q_ASSERT( items().contains( item ) );
    removeItemRectType( item );
    item->rect_type = type;
    resetItemRectType( item, type );
}

void ICGraphicsScene::clear() noexcept {
    for( const auto item: items() ) {
        if( item->type() == Qt::UserRole + 1 ) {
            removeAbstractItem( qgraphicsitem_cast< ICGraphicsItem * >( item ) );
        }
    }
}

void ICGraphicsScene::mousePressEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    if( event->button() == Qt::LeftButton && params_ic.isStarted() ) {
        end_point = start_point = event->scenePos();
        if( item ) removeAbstractItem( item );
        addAbstractItem( item = new ICGraphicsItem( QRectF{ start_point, end_point }.normalized() ), ICGraphicsItem::NONE );
        update();
    }
    QGraphicsScene::mousePressEvent( event );
}

void ICGraphicsScene::mouseMoveEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    QGraphicsScene::mouseMoveEvent( event );
    if( event->buttons() & Qt::LeftButton && params_ic.isStarted() ) {
        end_point = event->scenePos();
        //画框过渡矩形
        if( item )
            item->resetRect( QRectF( start_point, end_point ).normalized() );
        else
            addAbstractItem( item = new ICGraphicsItem( QRectF{ start_point, end_point }.normalized() ), ICGraphicsItem::NONE );
        update();
    }
}

void ICGraphicsScene::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    QGraphicsScene::mouseReleaseEvent( event );
    if( event->button() == Qt::LeftButton && params_ic.isStarted() ) {
        end_point        = event->scenePos();
        const auto &rect = QRectF{ start_point, end_point }.normalized();
        if( !item )
            return;
        //Item尺寸不符合要求或是Item数量够了，不添加当前Item
        if( rect.width() < 30 || rect.height() < 30 ) {
            removeAbstractItem( item );
        }
        item = nullptr;
        update();
    }
}
