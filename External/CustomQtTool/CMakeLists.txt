cmake_minimum_required(VERSION 3.0)
project(CustomQtTool)
#
set_property( GLOBAL APPEND PROPERTY EXTERNAL_INC_DIRS
        C:/3rdLibary/qttools-5.12.9/src/shared/qtpropertybrowser
        External/CustomQtTool
        External/CustomQtTool/QtPropertyBrowser
#        External/CustomQtTool/SynchronizedView
        )
#
file(GLOB Src
        "C:/3rdLibary/qttools-5.12.9/src/shared/qtpropertybrowser/*.cpp"
        "C:/3rdLibary/qttools-5.12.9/src/shared/qtpropertybrowser/*.h"
        "QtPropertyBrowser/*.cpp"
        "QtPropertyBrowser/*.h"
        "QtPropertyBrowser/*.ui"
#        "SynchronizedView/*.cpp"
#        "SynchronizedView/*.h"
#        "SynchronizedView/*.ui"
        )
#message("CustomQtTool Src : ${Src}")
set_property( GLOBAL APPEND PROPERTY EXTERNAL_SRCS ${Src})
