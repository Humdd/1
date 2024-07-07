INCLUDEPATH += $${PWD}/QtPropertyBrowser \
                $${PWD}/AbstractPaintingView
FORMS += \
    $${PWD}/QtPropertyBrowser/AbstractPropertyBrowserWindow.ui \
#    $${PWD}/AbstractPaintingView/AbstractPaintingView.ui

SOURCES += \
        $${PWD}/QtPropertyBrowser/AbstractPropertyBrowser.cpp \
        $${PWD}/QtPropertyBrowser/AbstractPropertyBrowserWindow.cpp \
        $${PWD}/QtPropertyBrowser/PropertyBrowserTemplate.cpp \
#        $${PWD}/AbstractPaintingView/AbstractPaintingView.cpp

HEADERS += \
        $${PWD}/QtPropertyBrowser/AbstractPropertyBrowser.h \
        $${PWD}/QtPropertyBrowser/AbstractPropertyBrowserWindow.h \
        $${PWD}/QtPropertyBrowser/PropertyBrowserTemplate.h \
#        $${PWD}/AbstractPaintingView/AbstractPaintingView.h

include(C:/3rdLibary/qttools-5.12.9/src/shared/qtpropertybrowser/qtpropertybrowser.pri)
