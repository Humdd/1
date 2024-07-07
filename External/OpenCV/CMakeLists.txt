cmake_minimum_required(VERSION 3.0)
project(OpenCV)

set_property( GLOBAL APPEND PROPERTY EXTERNAL_INC_DIRS
        C:/3rdLibary/OpenCV/include/opencv2
        C:/3rdLibary/OpenCV/include/
        External/OpenCV
        External/OpenCV/SupportClass
        )

set_property( GLOBAL APPEND PROPERTY EXTERNAL_LIBS_DIRS C:/3rdLibary/OpenCV/x64/vc16/lib )

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set_property( GLOBAL APPEND PROPERTY EXTERNAL_LIBS opencv_world454d opencv_img_hash454d )
elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_property( GLOBAL APPEND PROPERTY EXTERNAL_LIBS opencv_world454 opencv_img_hash454 )
endif()
file(GLOB Src
        "OpenCVHelp.*"
        "OpenCVOutput.h"
        "OpenCVSerialization.*"
        "SupportClass/Rotater.*"
        SupportClass/Cv2Qt.*
        )
#message("OpenCV Src : ${Src}")
set_property( GLOBAL APPEND PROPERTY EXTERNAL_SRCS ${Src})
