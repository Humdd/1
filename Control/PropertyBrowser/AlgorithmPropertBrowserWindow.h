#ifndef ALGORITHMPROPERTBROWSERWINDOW_H
#define ALGORITHMPROPERTBROWSERWINDOW_H

#include <AbstractPropertyBrowserWindow.h>

struct AlgorithmPropertBrowserWindow
    : public AbstractPropertyBrowserWindow
    , public SingletonTemplate< AlgorithmPropertBrowserWindow > {
protected:
    explicit AlgorithmPropertBrowserWindow( QWidget *parent = nullptr ) noexcept;
    Q_DECLARE_SINGLETON( AlgorithmPropertBrowserWindow )
};

#endif// ALGORITHMPROPERTBROWSERWINDOW_H
