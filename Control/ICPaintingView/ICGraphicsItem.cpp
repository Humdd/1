#include "ICGraphicsItem.h"
#include "ICGraphicsScene.h"
#include "ICAlgorithmParams.h"
#include "SupportClass/ICMenuHelper.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

ICGraphicsItem::ICGraphicsItem( const QRectF &rect, const int type ) noexcept
    : rect_type( type )
    , rect( rect )
    , params_rect( Cv2Qt::convertRect< cv::Rect >( rect ) )
    , color( getColor( type ) ) {
    setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable );
    setAcceptHoverEvents( true );
}

void ICGraphicsItem::setType( int type ) noexcept {
    Q_ASSERT( type >= 0 && type < 4 );
    if( type == rect_type )
        return;

    if( type != NONE ) {
        for( auto &elem: scene()->items() ) {
            if( elem->type() != Qt::UserRole + 1 )
                continue;
            else if( auto item = qgraphicsitem_cast< ICGraphicsItem * >( elem ); ( Q_ASSERT( item ), item->rect_type == type && item != this ) ) {
                //否则是其他矩形，先给予用户警告，让用户选择
                const auto ret = QMessageBox::critical(
                    nullptr, u8"警告",
                    QString( u8"type为%1的矩形已经存在！请问是否继续标注该矩形？\n"
                             "若选Yes，则旧矩形会被设为NONE！\n"
                             "若选No，则新矩形type不变！" )
                        .arg( getEnumStr( type ) ),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes );
                if( ret == QMessageBox::Yes )//选择Yes
                    item->setType( NONE );   //旧矩形设为NONE
                else                         //选择No
                    return;                  //直接返回即可
            }
        }
    }

    prepareGeometryChange();
    Q_ASSERT( qobject_cast< ICGraphicsScene * >( scene() ) );
    qobject_cast< ICGraphicsScene * >( scene() )->resetItemRectType( type, this );
    color = getColor( type );
}

void ICGraphicsItem::resetRect( const QRectF &rect ) noexcept {
    prepareGeometryChange();
    this->rect  = rect;
    params_rect = Cv2Qt::convertRect< cv::Rect >( rect );
    printVar( &params_rect );
    printVar( params_rect );
    printVar( getEnumStr( rect_type ) );
    printSeparate;
}

const char *ICGraphicsItem::getEnumStr( const int type ) noexcept {
    static constexpr std::array< const char *, 4 > dst = {
        "NONE",
        "RECT_CHIP",
        "RECT_LOGO",
        "RECT_CIRCLE"
    };
    Q_ASSERT( type >= 0 && type < 4 );
    return dst[ type ];
}

QColor ICGraphicsItem::getColor( const int type ) noexcept {
    static constexpr std::array< QColor, 4 > dst = {
        QColor( 255, 255, 255 ),
        QColor( 0, 255, 0 ),
        QColor( 0, 0, 255 ),
        QColor( 255, 0, 0 )
    };
    Q_ASSERT( type >= 0 && type < 4 );
    return dst[ type ];
}

QRectF ICGraphicsItem::boundingRect() const noexcept {
    return rect;
}

int ICGraphicsItem::type() const noexcept {
    return Qt::UserRole + 1;
}

void ICGraphicsItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * ) noexcept {
    if( rect.width() <= 0 || rect.height() <= 0 || !scene() ) return;
    Q_ASSERT( rect_type >= 0 && rect_type < 4 );
    painter->setPen( QPen( { color }, 5 ) );
    painter->drawRect( rect );
    scene()->update();//实时更新
}

cv::Rect *ICGraphicsItem::getParamRect() noexcept {
    printVar( &params_rect );
    printVar( params_rect );
    printVar( getEnumStr( rect_type ) );
    printSeparate;
    return &params_rect;
}

void ICGraphicsItem::mousePressEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    if( event->button() == Qt::MiddleButton ) {
        setCursor( QCursor( Qt::ClosedHandCursor ) );
    }
    QGraphicsItem::mousePressEvent( event );
}

void ICGraphicsItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    if( event->buttons() & Qt::MiddleButton ) {
        //计算鼠标移动的偏移量
        qreal dx = event->scenePos().x() - event->lastScenePos().x();
        qreal dy = event->scenePos().y() - event->lastScenePos().y();
        //重新生成矩形框
        resetRect( { rect.x() + dx, rect.y() + dy, rect.width(), rect.height() } );
    }
}

void ICGraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    QGraphicsItem::mouseReleaseEvent( event );
    if( event->button() == Qt::RightButton ) {
        printFuncInfo;
        static ICMenuHelper menu_helper;
        const auto          select_index = menu_helper.getSelectedIndex( event->screenPos() );
        Q_ASSERT( select_index >= -2 && select_index <= 4 );
        switch( select_index ) {
            case -2: break;
            case -1:
                Q_ASSERT( qobject_cast< ICGraphicsScene * >( scene() ) );
                qobject_cast< ICGraphicsScene * >( scene() )->removeAbstractItem( this );
                break;
            case 4:
                printFuncInfo;
                if( rect_type == RECT_CHIP || rect_type == RECT_LOGO || rect_type == RECT_CIRCLE )
                    params_ic.rect_binary_threshold_ptr = &std::get< 1 >( params_ic.rect_tuple_array[ rect_type - 1 ] );
                break;
            default:
                setType( select_index );
        }
    }
    else if( event->button() == Qt::MiddleButton ) {
        setCursor( QCursor( Qt::OpenHandCursor ) );
    }
}

void ICGraphicsItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    printFuncInfo;
    if( event->button() == Qt::LeftButton ) {
        Q_ASSERT( qobject_cast< ICGraphicsScene * >( scene() ) );
        qobject_cast< ICGraphicsScene * >( scene() )->removeAbstractItem( this );
    }
    QGraphicsItem::mouseDoubleClickEvent( event );
}
