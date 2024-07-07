QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

CONFIG(release, debug|release){
#DEFINES += QT_NO_DEBUG_OUTPUT #Release模式下禁止QDebug
QMAKE_CXXFLAGS_RELEASE += -O3      # Release -O2 -Wall
}
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += \
    $$PWD \
    $$PWD/inc \
    $$PWD/Control \
    $$PWD/Control/AbstractPaintingView \
    $$PWD/Control/ICPaintingView \
    $$PWD/Control/PropertyBrowser \
    $$PWD/SupportClass \
    Windows

DEPENDPATH += $$PWD/inc

HEADERS += \
    CDeviceProcess.h \
    CProcess.h \
    Client.h \
    Control/AbstractPaintingView/AbstractGraphicsItem.h \
    Control/AbstractPaintingView/AbstractGraphicsScene.h \
    Control/AbstractPaintingView/AbstractPaintingView.h \
    Control/PropertyBrowser/AlgorithmPropertBrowser.h \
    Control/PropertyBrowser/AlgorithmPropertBrowserWindow.h \
    Control/PropertyBrowser/ICAlgorithmPropertBrowser.h \
    Control/PropertyBrowser/ICAlgorithmPropertBrowserWindow.h \
    Control/ICPaintingView/ICGraphicsItem.h \
    Control/ICPaintingView/ICGraphicsScene.h \
    Control/ICPaintingView/ICPaintingView.h \
    SupportClass/AlgorithmParams.h \
    SupportClass/CircularArrays.h \
    SupportClass/MenuHelper.h \
    SupportClass/ICAlgorithmParams.h \
    SupportClass/ICCircularArrays.h \
    SupportClass/ICMenuHelper.h \
    Windows/FilmStripWindow.h \
    Windows/ICWindow.h \
    FilmStripClassifier.h \
    ICClassifier.h \
    DisplayImageWidget.h \
    cgxbitmap.h \
    comboboxtest.h \
    header.h \
    mainwindow.h \
    tcp_server.h \
    tcp_server_private.h \
    utils.h

SOURCES += \
    CDeviceProcess.cpp \
    Control/AbstractPaintingView/AbstractGraphicsItem.cpp \
    Control/AbstractPaintingView/AbstractGraphicsScene.cpp \
    Control/AbstractPaintingView/AbstractPaintingView.cpp \
    Control/PropertyBrowser/AlgorithmPropertBrowser.cpp \
    Control/PropertyBrowser/AlgorithmPropertBrowserWindow.cpp \
    Control/PropertyBrowser/ICAlgorithmPropertBrowser.cpp \
    Control/PropertyBrowser/ICAlgorithmPropertBrowserWindow.cpp \
    Control/ICPaintingView/ICGraphicsItem.cpp \
    Control/ICPaintingView/ICGraphicsScene.cpp \
    Control/ICPaintingView/ICPaintingView.cpp \
    SupportClass/AlgorithmParams.cpp \
    SupportClass/CircularArrays.cpp \
    SupportClass/MenuHelper.cpp \
    SupportClass/ICAlgorithmParams.cpp \
    SupportClass/ICCircularArrays.cpp \
    SupportClass/ICMenuHelper.cpp \
    Windows/ICWindow.cpp \
    ICClassifier.cpp \
    Windows/FilmStripWindow.cpp \
    FilmStripClassifier.cpp \
    DisplayImageWidget.cpp \
    cgxbitmap.cpp \
    comboboxtest.cpp \
    main.cpp \
    mainwindow.cpp \
    tcp_server.cpp \
    tcp_server_private.cpp \
    utils.cpp


FORMS += \
    Control/AbstractPaintingView/AbstractPaintingView.ui \
    Control/ICPaintingView/ICPaintingView.ui \
    External/CustomQtTool/AbstractPaintingView/AbstractPaintingView.ui \
    External/CustomQtTool/QtPropertyBrowser/AbstractPropertyBrowserWindow.ui \
    External/CustomQtTool/SynchronizedView/SynchronizedView.ui \
    Windows/FilmStripWindow.ui \
    Windows/ICWindow.ui \
    cgxbitmap.ui \
    comboboxtest.ui \
    mainwindow.ui


#include(C:/LoadPri.pri)
include(Log4cplus/Log4cplus.pri)
#include(C:/Program_Sources-backup/Lib_Dll/OpenCV.pri)
#include("C:/Program_Sources/Lib_Dll/External/OpenCV/OpenCV.pri")
include(External/External.pri)

TARGET = bdj
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


LIBS += -L$$PWD/lib/x64/ -lGxIAPICPPEx

#INCLUDEPATH += $$PWD/lib/x64
#DEPENDPATH += $$PWD/lib/x64

#unix|win32: LIBS += -L'C:/Program Files/Daheng Imaging/GalaxySDK/Samples/C++ SDK/lib/x64/' -lGxIAPICPPEx

#INCLUDEPATH += 'C:/Program Files/Daheng Imaging/GalaxySDK/Samples/C++ SDK/lib/x64'
#DEPENDPATH += 'C:/Program Files/Daheng Imaging/GalaxySDK/Samples/C++ SDK/lib/x64'
