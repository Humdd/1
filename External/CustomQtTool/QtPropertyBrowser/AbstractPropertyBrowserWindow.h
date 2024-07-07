#ifndef QTTREEBROWSERWINDOW_H
#define QTTREEBROWSERWINDOW_H

#include "AbstractPropertyBrowser.h"
#include <QMainWindow>

namespace Ui {
    class AbstractPropertyBrowserWindow;
}

class AbstractPropertyBrowserWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit AbstractPropertyBrowserWindow( AbstractPropertyBrowser *browser, QWidget *parent = nullptr ) noexcept;

    explicit AbstractPropertyBrowserWindow( QWidget *parent = nullptr ) noexcept;

    ~AbstractPropertyBrowserWindow();

    AbstractPropertyBrowser *getPropertyBrowser() noexcept;

    template< typename SubClass >
    SubClass *getSubClassBrowser() noexcept {
        static SubClass *dst = qobject_cast< SubClass * >( getPropertyBrowser() );
        assert( dst );
        return dst;
    }

protected:
    void connectToQtTreeBrowser( AbstractPropertyBrowser *browser ) noexcept;

    AbstractPropertyBrowser *browser = nullptr;

    Ui::AbstractPropertyBrowserWindow *ui;
};

#endif// QTTREEBROWSERWINDOW_H
