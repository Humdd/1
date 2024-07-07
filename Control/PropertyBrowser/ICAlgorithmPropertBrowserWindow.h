#ifndef ICALGORITHMPROPERTBROWSERWINDOW_H
#define ICALGORITHMPROPERTBROWSERWINDOW_H

#include <AbstractPropertyBrowserWindow.h>

struct ICAlgorithmPropertBrowserWindow
    : public AbstractPropertyBrowserWindow
    , public SingletonTemplate< ICAlgorithmPropertBrowserWindow > {
protected:
    explicit ICAlgorithmPropertBrowserWindow( QWidget *parent = nullptr ) noexcept;
    Q_DECLARE_SINGLETON( ICAlgorithmPropertBrowserWindow )
};

#endif// ICALGORITHMPROPERTBROWSERWINDOW_H
