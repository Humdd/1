#include "MenuHelper.h"

MenuHelper::MenuHelper() noexcept {
    action_array = {
        menu.addAction( u8"删除元素" ),
        menu.addAction( u8"取消元素状态" ),
        menu.addAction( u8"标志为上矩形" ),
        menu.addAction( u8"标志为下矩形" ),
        menu.addAction( u8"标志为小圆圈矩形" ),
        menu.addAction( u8"调整二值化阈值" )
    };
}

int MenuHelper::getSelectedIndex( const QPointF &pos ) noexcept {
    const auto action = menu.exec( pos.toPoint() );
    if( !action )
        return -2;
    else
        return action_array.indexOf( action ) - 1;
}
