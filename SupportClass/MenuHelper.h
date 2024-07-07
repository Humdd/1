#ifndef MENUHELPER_H
#define MENUHELPER_H
#include <QVarLengthArray>
#include <QMenu>

struct MenuHelper {
    MenuHelper() noexcept;
    int getSelectedIndex( const QPointF &pos ) noexcept;

private:
    QMenu                           menu;
    QVarLengthArray< QAction *, 6 > action_array;
};

#endif// MENUHELPER_H
