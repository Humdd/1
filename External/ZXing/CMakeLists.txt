cmake_minimum_required(VERSION 3.0)
project(ZXing)

set_property( GLOBAL APPEND PROPERTY EXTERNAL_INC_DIRS
        C:/3rdLibary/ZXing/src
        C:/3rdLibary/ZXing/example
        )

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set_property( GLOBAL APPEND PROPERTY EXTERNAL_LIBS_DIRS C:/3rdLibary/ZXing/build/core/Debug )
elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_property( GLOBAL APPEND PROPERTY EXTERNAL_LIBS_DIRS C:/3rdLibary/ZXing/build/core/Release )
endif()
set_property( GLOBAL APPEND PROPERTY EXTERNAL_LIBS ZXing )

file(GLOB Src
        "C:/3rdLibary/ZXing/example/ZXingOpenCV.h"
        )

#message("ZXing Src : ${Src}")
set_property( GLOBAL APPEND PROPERTY EXTERNAL_SRCS ${Src})
