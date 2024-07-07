cmake_minimum_required(VERSION 3.0)
project(spdlog)

set_property( GLOBAL APPEND PROPERTY EXTERNAL_INC_DIRS
        C:/3rdLibary/spdlog/include
        External/spdlog
        )

file(GLOB Src
#        "QtSink.*"
#        "EncryptedRotatingFileSink.*"
        "SpdLogger.*"
        )
#message("spdlog Src : ${Src}")
set_property( GLOBAL APPEND PROPERTY EXTERNAL_SRCS ${Src})
