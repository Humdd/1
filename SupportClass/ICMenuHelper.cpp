#include "ICMenuHelper.h"

ICMenuHelper::ICMenuHelper() noexcept {
    action_array = {
        menu.addAction( u8"删除元素" ),
        menu.addAction( u8"取消元素状态" ),
        menu.addAction( u8"标记为芯片" ),
        menu.addAction( u8"标记为Logo" ),
        menu.addAction( u8"标记为白色圆点矩形" ),
        menu.addAction( u8"调整二值化阈值" )
    };
}

int ICMenuHelper::getSelectedIndex( const QPointF &pos ) noexcept {
    const auto action = menu.exec( pos.toPoint() );
    if( !action )
        return -2;
    else
        return action_array.indexOf( action ) - 1;
}
