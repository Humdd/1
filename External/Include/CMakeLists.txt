cmake_minimum_required(VERSION 3.0)
project(Include)
set_property( GLOBAL APPEND PROPERTY EXTERNAL_INC_DIRS
        C:/3rdLibary/Include
        C:/3rdLibary/Include/SupportClass
        C:/3rdLibary/Include/MathUtils
        )

file(GLOB Src
#        "C:/3rdLibary/Include/CustomStackWalker.*"
#        "C:/3rdLibary/Include/SupportClass/StackWalker.cpp"
#        "C:/3rdLibary/Include/ClassFactory.*"
        "C:/3rdLibary/Include/SupportClass/fmtStr.h"
        "C:/3rdLibary/Include/SupportClass/MyTypeTraits.h"
        "C:/3rdLibary/Include/MyMacros.*"
        "C:/3rdLibary/Include/TimeRecorder.*"
        "C:/3rdLibary/Include/JsonHelper.*"
        "C:/3rdLibary/Include/DirManager.*"
        "C:/3rdLibary/Include/DataStorage.*"
        "C:/3rdLibary/Include/ContainerOperation.*"
        "C:/3rdLibary/Include/StringOperation.*"
        "C:/3rdLibary/Include/SingletonTemplate.*"        
        )
#message("Include Src : ${Src}")
set_property( GLOBAL APPEND PROPERTY EXTERNAL_SRCS ${Src})
