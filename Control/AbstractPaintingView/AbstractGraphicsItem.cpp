#include "AbstractGraphicsItem.h"
#include "AbstractGraphicsScene.h"
#include "AlgorithmParams.h"
#include "SupportClass/MenuHelper.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

//主要跟图上的标注事件相关
AbstractGraphicsItem::AbstractGraphicsItem( const QRectF &rect, const int type ) noexcept
    : rect_type( type )
    , rect( rect )
    , params_rect( Cv2Qt::convertRect< cv::Rect >( rect ) )
    , color( getColor( type ) ) {
    //设置该图形项的属性
    //ItemIsSelectable 表示图形项可以被选择，通常是使用鼠标进行选择操作；
    //ItemIsMovable 表示图形项可以被移动，通常是使用鼠标进行拖拽操作；
    //ItemIsFocusable 表示图形项可以被设置为焦点状态，通常是使用键盘进行焦点切换操作。
    setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable );

    //ItemIsFocusable 表示图形项可以被设置为焦点状态，通常是使用键盘进行焦点切换操作。
    setAcceptHoverEvents( true );
}

//设置选框类型
void AbstractGraphicsItem::setType( int type ) noexcept {
    Q_ASSERT( type >= 0 && type < 4 );
    if( type == rect_type )
        return;
    //查询type类型的矩形是否已经存在 （RECT_UP || RECT_DOWN）
    if( type == RECT_UP || type == RECT_DOWN ) {
        for( auto &elem : scene()->items() ) {
            if( auto item = qgraphicsitem_cast< AbstractGraphicsItem * >( elem ); ( Q_ASSERT( item ), item->rect_type == type ) ) {
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
    Q_ASSERT( qobject_cast< AbstractGraphicsScene * >( scene() ) );
    qobject_cast< AbstractGraphicsScene * >( scene() )->resetItemRectType( type, this );
    color = getColor( type );
}

//重新进行选框
void AbstractGraphicsItem::resetRect( const QRectF &rect ) noexcept {
    prepareGeometryChange();
    this->rect  = rect;
    params_rect = Cv2Qt::convertRect< cv::Rect >( rect );
    printVar( rect );
    printVar( params_rect );
    printSeparate;
}

//选择标注类型
const char *AbstractGraphicsItem::getEnumStr( const int type ) noexcept {
    static constexpr std::array< const char *, 4 > dst = {
        "NONE",
        "RECT_UP",
        "RECT_DOWN",
        "CIRCLE"
    };
    Q_ASSERT( type >= 0 && type < 4 );
    return dst[ type ];
}

//选择标注颜色
QColor AbstractGraphicsItem::getColor( const int type ) noexcept {
    static constexpr std::array< QColor, 4 > dst = {
        QColor( 255, 255, 255 ),
        QColor( 0, 255, 0 ),
        QColor( 0, 0, 255 ),
        QColor( 255, 0, 0 )
    };
    Q_ASSERT( type >= 0 && type < 4 );
    return dst[ type ];
}

QRectF AbstractGraphicsItem::boundingRect() const noexcept {
    return rect;
}

int AbstractGraphicsItem::type() const noexcept {
    return Qt::UserRole + 1;
}

void AbstractGraphicsItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * ) noexcept {
    if( rect.width() <= 0 || rect.height() <= 0 || !scene() ) return;
    Q_ASSERT( rect_type >= 0 && rect_type < 4 );
    painter->setPen( QPen( { color }, 5 ) );
    painter->drawRect( rect );
    scene()->update();//实时更新
}

cv::Rect *AbstractGraphicsItem::getParamRect() noexcept {
    printFuncInfo;
    return &params_rect;
}
//鼠标点击
void AbstractGraphicsItem::mousePressEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    if( event->button() == Qt::MiddleButton ) {
        setCursor( QCursor( Qt::ClosedHandCursor ) );
    }
    QGraphicsItem::mousePressEvent( event );
}
//鼠标移动
void AbstractGraphicsItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    if( event->buttons() & Qt::MiddleButton ) {
        //计算鼠标移动的偏移量
        qreal dx = event->scenePos().x() - event->lastScenePos().x();
        qreal dy = event->scenePos().y() - event->lastScenePos().y();
        //重新生成矩形框
        resetRect( { rect.x() + dx, rect.y() + dy, rect.width(), rect.height() } );
    }
    QGraphicsItem::mouseMoveEvent( event );
}

//鼠标释放
void AbstractGraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    printFuncInfo;
    QGraphicsItem::mouseReleaseEvent( event );
    if( event->button() == Qt::RightButton ) {
        static MenuHelper menu_helper;
        const auto        select_index = menu_helper.getSelectedIndex( event->screenPos() );
        Q_ASSERT( select_index >= -2 && select_index <= 4 );
        switch( select_index ) {
            case -2: break;
            case -1:
                Q_ASSERT( qobject_cast< AbstractGraphicsScene * >( scene() ) );
                qobject_cast< AbstractGraphicsScene * >( scene() )->removeAbstractItem( this );
                break;
            case 4:
                printFuncInfo;
                if( rect_type == RECT_UP || rect_type == RECT_DOWN )
                    params.rect_binary_threshold_ptr = &std::get< 1 >( params.rect_tuple_array[ rect_type - 1 ] );
                break;
            default:
                setType( select_index );
        }
    }
    else if( event->button() == Qt::MiddleButton ) {
        setCursor( QCursor( Qt::OpenHandCursor ) );
    }
}

//双击鼠标
void AbstractGraphicsItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) noexcept {
    printFuncInfo;
    if( event->button() == Qt::LeftButton ) {
        Q_ASSERT( qobject_cast< AbstractGraphicsScene * >( scene() ) );
        qobject_cast< AbstractGraphicsScene * >( scene() )->removeAbstractItem( this );
    }
    QGraphicsItem::mouseDoubleClickEvent( event );
}
