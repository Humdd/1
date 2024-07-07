#ifndef ICMENUHELPER_H
#define ICMENUHELPER_H
#include <QVarLengthArray>
#include <QMenu>

struct ICMenuHelper {
    ICMenuHelper() noexcept;
    int getSelectedIndex( const QPointF &pos ) noexcept;

private:
    QMenu                           menu;
    QVarLengthArray< QAction *, 6 > action_array;
};

#endif// ICMENUHELPER_H
