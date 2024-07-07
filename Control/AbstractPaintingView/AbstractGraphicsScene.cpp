#include "AbstractGraphicsScene.h"
#include "AbstractGraphicsItem.h"
#include "AlgorithmParams.h"
#include <QGraphicsSceneMouseEvent>

AbstractGraphicsScene::AbstractGraphicsScene( QObject *parent ) noexcept
    : QGraphicsScene{ parent } {
    item_array.reserve( 8 );
}

void AbstractGraphicsScene::removeItemRectType( AbstractGraphicsItem *item ) noexcept {
    printFuncInfo;
    Q_ASSERT( item );
    Q_ASSERT( item->rect_type >= 0 && item->rect_type < 4 );
    switch( item->rect_type ) {
        case AbstractGraphicsItem::NONE:
            break;
        case AbstractGraphicsItem::RECT_UP:
            std::get< 0 >( params.rect_tuple_array[ 0 ] ) = nullptr;
            break;
        case AbstractGraphicsItem::RECT_DOWN:
            std::get< 0 >( params.rect_tuple_array[ 1 ] ) = nullptr;
            break;
        case AbstractGraphicsItem::CIRCLE:
            params.rect_circle_array.removeOne( item->getParamRect() );
            break;
    }
//    resetItemRectType( item, AbstractGraphicsItem::NONE );
}

void AbstractGraphicsScene::resetItemRectType( AbstractGraphicsItem *item, const int type ) noexcept {
    printFuncInfo;
    Q_ASSERT( item );
    Q_ASSERT( type >= 0 && type < 4 );
//    for( auto &[ rect_ptr, _1, _2, _3 ]: params_ic.rect_tuple_array ) {
//        if( rect_ptr == item->getParamRect() ) {
//            rect_ptr = nullptr;
//            break;
//        }
//    }
    switch( type ) {
        case AbstractGraphicsItem::NONE:
            break;
        case AbstractGraphicsItem::RECT_UP:
            std::get< 0 >( params.rect_tuple_array[ 0 ] ) = item->getParamRect();
            break;
        case AbstractGraphicsItem::RECT_DOWN:
            std::get< 0 >( params.rect_tuple_array[ 1 ] ) = item->getParamRect();
            break;
        case AbstractGraphicsItem::CIRCLE:
            //            std::get< 0 >( params.rect_tuple_array[ 2 ] ) = item->getParamRect();
            params.rect_circle_array.append( item->getParamRect() );
            break;
    }
    //    if( type != AbstractGraphicsItem::NONE )
    //        std::get< 0 >( params_ic.rect_tuple_array[ type - 1 ] ) = item->getParamRect();
}

void AbstractGraphicsScene::removeAbstractItem( AbstractGraphicsItem *item ) noexcept {
    printFuncInfo;
    Q_ASSERT( items().contains( item ) );
    removeItemRectType( item );
    //查找item所处位置的迭代器 因为已经断言items().contains( item )， 所以iter必定有效
    auto iter = std::find_if( item_array.begin(), item_array.end(), [ item ]( auto &&ptr ) {
        return ptr.get() == item;
    } );
    item_array.erase( iter );//数组删除item，智能指针能让item自动析构,item析构会自己从scene中删除
}

void AbstractGraphicsScene::addAbstractItem( AbstractGraphicsItem *item, const int type ) noexcept {
    printFuncInfo;
    Q_ASSERT( !items().contains( item ) );
    item_array.emplace_back( item );
    resetItemRectType( item, type );
    QGraphicsScene::addItem( item );
}

void AbstractGraphicsScene::resetItemRectType( const int type, AbstractGraphicsItem *item ) noexcept {
    printFuncInfo;
    Q_ASSERT( type >= 0 && type < 4 );
    Q_ASSERT( item );
    Q_ASSERT( items().contains( item ) );
    removeItemRectType( item );
    item->rect_type = type;
    resetItemRectType( item, type );
}

void AbstractGraphicsScene::clear() noexcept {
    for( const auto item: items() ) {
        if( item->type() == Qt::UserRole + 1 ) {
            removeAbstractItem( qgraphicsitem_cast< AbstractGraphicsItem * >( item ) );
        }
    }
}

void AbstractGraphicsScene::mousePressEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    if( event->button() == Qt::LeftButton && params.isStarted() ) {
        end_point = start_point = event->scenePos();
        if( item ) removeAbstractItem( item );
        addAbstractItem( item = new AbstractGraphicsItem( QRectF{ start_point, end_point }.normalized() ) );
        update();
    }
    QGraphicsScene::mousePressEvent( event );
}

void AbstractGraphicsScene::mouseMoveEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    QGraphicsScene::mouseMoveEvent( event );
    if( event->buttons() & Qt::LeftButton && params.isStarted() ) {
        end_point = event->scenePos();
        //画框过渡矩形
        if( item )
            item->resetRect( QRectF( start_point, end_point ).normalized() );
        else
            addAbstractItem( item = new AbstractGraphicsItem( QRectF{ start_point, end_point }.normalized() ) );
        update();
    }
}

void AbstractGraphicsScene::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    QGraphicsScene::mouseReleaseEvent( event );
    if( event->button() == Qt::LeftButton && params.isStarted() ) {
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
